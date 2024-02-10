// file   : src/Room.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Room.hpp"

#include "Session.hpp"
#include "Player.hpp"
#include "Bot.hpp"
#include "User.hpp"

#include "entity/Avatar.hpp"
#include "entity/Food.hpp"
#include "entity/Mass.hpp"
#include "entity/Virus.hpp"
#include "entity/Phage.hpp"
#include "entity/Mother.hpp"

#include "geometry/geometry.hpp"

#include "packet/EmptyPacket.hpp"
#include "packet/OutputPacketTypes.hpp"
#include "packet/PacketLeaderboard.hpp"
#include "packet/PacketPlay.hpp"
#include "packet/PacketSpectate.hpp"
#include "packet/serialization.hpp"

#include <boost/asio/post.hpp>

#include <spdlog/spdlog.h>
#include <chrono>

using namespace std::chrono;

Room::Room(asio::any_io_executor executor, uint32_t id)
  : m_executor(std::move(executor))
  , m_updateTimer(m_executor, std::bind_front(&Room::update, this))
  , m_checkPlayersTimer(m_executor, std::bind_front(&Room::checkPlayers, this))
  , m_updateLeaderboardTimer(m_executor, std::bind_front(&Room::updateLeaderboard, this))
  , m_destroyOutdatedCellsTimer(m_executor, std::bind_front(&Room::destroyOutdatedCells, this))
  , m_checkMothersTimer(m_executor, std::bind_front(&Room::checkMothers, this))
  , m_produceMothersTimer(m_executor, std::bind_front(&Room::produceMothers, this))
  , m_id(id)
{
}

Room::~Room()
{
  for (const auto& it : m_players) {
    delete it.second;
  }
  for (auto* it : m_avatarContainer) {
    delete it;
  }
  for (auto* it : m_foodContainer) {
    delete it;
  }
  for (auto* it : m_massContainer) {
    delete it;
  }
  for (auto* it : m_virusContainer) {
    delete it;
  }
  for (auto* it : m_phageContainer) {
    delete it;
  }
  for (auto* it : m_motherContainer) {
    delete it;
  }
}

void Room::init(const RoomConfig& config)
{
  m_config = config;

  m_updateTimer.setInterval(m_config.updateInterval);
  m_checkPlayersTimer.setInterval(m_config.checkPlayersInterval);
  m_updateLeaderboardTimer.setInterval(m_config.updateLeaderboardInterval);
  m_destroyOutdatedCellsTimer.setInterval(m_config.destroyOutdatedCellsInterval);
  m_checkMothersTimer.setInterval(m_config.checkMothersInterval);
  m_produceMothersTimer.setInterval(m_config.produceMothersInterval);

  m_simulationInterval = duration_cast<duration<double>>(m_config.updateInterval).count();
  m_simulationInterval /= m_config.simulationsPerUpdate;

  m_cellMinRadius = m_config.cellRadiusRatio * sqrt(m_config.cellMinMass / M_PI);
  m_cellMaxRadius = m_config.cellRadiusRatio * sqrt(m_config.maxMass / M_PI);
  m_cellRadiusDiff = m_cellMaxRadius - m_cellMinRadius;
  m_avatarSpeedDiff = m_config.avatarMaxSpeed - m_config.avatarMinSpeed;

  m_gridmap.resize(m_config.width, m_config.height, 9);
  spawnFood(m_config.foodStartAmount);
  spawnViruses(m_config.virusStartAmount);
  spawnPhages(m_config.phageStartAmount);
  spawnMothers(m_config.motherStartAmount);

  uint32_t id = 100;
  for (const auto& name : m_config.botNames) {
    spawnBot(id++, name);
  }
}

void Room::start()
{
  m_updateTimer.start();
  m_checkPlayersTimer.start();
  m_updateLeaderboardTimer.start();
  m_destroyOutdatedCellsTimer.start();
  m_checkMothersTimer.start();
  m_produceMothersTimer.start();
}

void Room::stop()
{
  m_updateTimer.stop();
  m_checkPlayersTimer.stop();
  m_updateLeaderboardTimer.stop();
  m_destroyOutdatedCellsTimer.stop();
  m_checkMothersTimer.stop();
  m_produceMothersTimer.stop();
}

uint32_t Room::getId() const
{
  return m_id;
}

bool Room::hasFreeSpace() const
{
  return m_hasFreeSpace;
}

const RoomConfig& Room::getConfig() const
{
  return m_config;
}

void Room::join(const SessionPtr& sess)
{
  asio::post(m_executor, std::bind_front(&Room::doJoin, this, sess));
}

void Room::leave(const SessionPtr& sess)
{
  asio::post(m_executor, std::bind_front(&Room::doLeave, this, sess));
}

void Room::play(const SessionPtr& sess, const std::string& name, uint8_t color)
{
  asio::post(m_executor, std::bind_front(&Room::doPlay, this, sess, name, color));
}

void Room::spectate(const SessionPtr& sess, uint32_t targetId)
{
  asio::post(m_executor, std::bind_front(&Room::doSpectate, this, sess, targetId));
}

void Room::point(const SessionPtr& sess, const Vec2D& point)
{
  asio::post(m_executor, std::bind_front(&Room::doPoint, this, sess, point));
}

void Room::eject(const SessionPtr& sess, const Vec2D& point)
{
  asio::post(m_executor, std::bind_front(&Room::doEject, this, sess, point));
}

void Room::split(const SessionPtr& sess, const Vec2D& point)
{
  asio::post(m_executor, std::bind_front(&Room::doSplit, this, sess, point));
}

void Room::watch(const SessionPtr& sess, uint32_t playerId)
{
  asio::post(m_executor, std::bind_front(&Room::doWatch, this, sess, playerId));
}

void Room::chatMessage(const SessionPtr& sess, const std::string& text)
{
  asio::post(m_executor, std::bind_front(&Room::doChatMessage, this, sess, text));
}

void Room::doJoin(const SessionPtr& sess)
{
  if (!m_sessions.emplace(sess).second) {
    return;
  }

  const auto& user = sess->user();
  if (!user) {
    throw std::runtime_error("Room::join(): the user doesn't set");
  }

  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::Room));
  serialize(*buffer, static_cast<uint16_t>(m_config.width));
  serialize(*buffer, static_cast<uint16_t>(m_config.height));
  serialize(*buffer, static_cast<uint16_t>(m_config.viewportBase));
  serialize(*buffer, m_config.viewportBuffer);
  serialize(*buffer, m_config.aspectRatio);
  serialize(*buffer, m_config.resistanceRatio);
  serialize(*buffer, m_config.elasticityRatio);
  serialize(*buffer, m_config.foodResistanceRatio);
  auto count = m_players.size();
  if (count > 255) {
    spdlog::warn("The number of players exceeds the limit of 255");
  }
  serialize(*buffer, static_cast<uint8_t>(count));
  for (const auto& it : m_players) {
    Player* player = it.second;
    serialize(*buffer, player->getId());
    serialize(*buffer, player->name);
    serialize(*buffer, player->getStatus());
  }
  count = m_chatHistory.size();
  if (count > 255) {
    spdlog::warn("The size of chat history exceeds the limit of 255");
  }
  serialize(*buffer, static_cast<uint8_t>(count));
  for (const auto& msg  : m_chatHistory) {
    serialize(*buffer, msg.authorId);
    serialize(*buffer, msg.author);
    serialize(*buffer, msg.text);
  }

  uint32_t playerId = 0;
  const auto& it = m_players.find(user->getId());
  if (it != m_players.end()) {
    Player* player = it->second;
    player->online = true;
    if (player->isDead()) {
      EmptyPacket packet(OutputPacketTypes::Finish);
      packet.format(*buffer);
    } else {
      player->addSession(sess);
      sess->player(player);
      PacketPlay packetPlay(*player);
      packetPlay.format(*buffer);
    }
    playerId = player->getId();
  } else {
    EmptyPacket packet(OutputPacketTypes::Finish);
    packet.format(*buffer);
  }
  sess->send(buffer);
  if (playerId) {
    sendPacketPlayerJoin(playerId);
    m_updateLeaderboard = true;
  }
}

void Room::doLeave(const SessionPtr& sess)
{
  if (m_sessions.erase(sess)) {
    m_pointerRequests.erase(sess);
    m_ejectRequests.erase(sess);
    m_splitRequests.erase(sess);
    auto* player = sess->player();
    if (player) {
      sendPacketPlayerLeave(player->getId());
      sess->player(nullptr);
      m_zombiePlayers.insert(player);
      player->online = false;
    }
    for (const auto& it : m_players) {
      it.second->removeSession(sess);
    }
  }
}

void Room::doPlay(const SessionPtr& sess, const std::string& name, uint8_t color)
{
  const auto& user = sess->user();
  if (!user) {
    throw std::runtime_error("Room::play(): the user doesn't set");
  }

  uint32_t playerId = user->getId();
  auto* player = sess->player();
  if (!player) {
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = new Player(playerId, *this, m_gridmap);
      player->name = name;
      m_players.emplace(playerId, player);
      recalculateFreeSpace();
      sendPacketPlayer(*player);
    }
    player->online = true;
    sess->player(player);
    sendPacketPlayerJoin(player->getId());
  }

  if (!player->isDead()) {
    return;
  }

  auto* observable = sess->observable();
  if (observable) {
    observable->removeSession(sess);
    sess->observable(nullptr);
  }

  player->init();
  player->wakeUp();
  if (player->name != name) {
    player->name = name;
    sendPacketPlayer(*player);
  }
  sendPacketPlayerBorn(player->getId());

  auto& obj = createAvatar();
  modifyMass(obj, m_config.avatarStartMass);
  int radius = obj.radius;
  obj.position.x = (m_generator() % (m_config.width - 2 * radius)) + radius;
  obj.position.y = (m_generator() % (m_config.height - 2 * radius)) + radius;
  obj.color = color;
  player->addAvatar(&obj);
  player->calcParams();
  m_leaderboard.emplace_back(player);
  m_updateLeaderboard = true;
  m_fighters.insert(player);
  const auto& buffer = std::make_shared<Buffer>();
  PacketPlay packetPlay(*player);
  packetPlay.format(*buffer);
  sess->send(buffer);
  player->addSession(sess); // TODO: розібратись з додаваннями і забираннями sess
}

void Room::doSpectate(const SessionPtr& sess, uint32_t targetId)
{
  const auto& user = sess->user();
  if (!user) {
    throw std::runtime_error("Room::spectate(): the user doesn't set");
  }

  uint32_t playerId = user->getId();
  auto* player = sess->player();
  if (!player) {
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = new Player(playerId, *this, m_gridmap);
      player->name = "Player " + std::to_string(user->getId());
      m_players.emplace(playerId, player);
      recalculateFreeSpace();
      sendPacketPlayer(*player);
    }
    player->online = true;
    sess->player(player);
    sendPacketPlayerJoin(player->getId());
  }

  if (!player->isDead()) {
    return;
  }

  Player* target = nullptr;
  const auto& it = m_players.find(targetId);
  if (it != m_players.end()) {
    target = it->second;
  } else if (!m_leaderboard.empty()) {
    target = m_leaderboard.at(0);
  }
  if (m_fighters.find(target) == m_fighters.end()) {
    return;
  }
  if (player == target || sess->observable() == target) {
    return;
  }
  if (player) {
    player->removeSession(sess);
  }
  if (auto observable = sess->observable()) {
    observable->removeSession(sess);
  }
  const auto& buffer = std::make_shared<Buffer>();
  PacketSpectate packet(*target);
  packet.format(*buffer);
  sess->send(buffer);
  target->addSession(sess);
  sess->observable(target);
}

void Room::doPoint(const SessionPtr& sess, const Vec2D& point)
{
  m_pointerRequests.emplace(sess, point);
}

void Room::doEject(const SessionPtr& sess, const Vec2D& point)
{
  m_ejectRequests.emplace(sess, point);
}

void Room::doSplit(const SessionPtr& sess, const Vec2D& point)
{
  m_splitRequests.emplace(sess, point);
}

void Room::doWatch(const SessionPtr& sess, uint32_t playerId)
{
  auto* player = sess->player();
  if (!player) {
    return;
  }
  const auto& it = m_players.find(playerId);
  if (it != m_players.end()) {
    auto* target = it->second;
    if (target != player) {
      player->arrowPlayer = target;
    }
  }
}

void Room::doChatMessage(const SessionPtr& sess, const std::string& text)
{
  auto* player = sess->player();
  if (!player){
    return;
  }
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::ChatMessage));
  serialize(*buffer, player->getId());
  serialize(*buffer, text);
  if (text[0] == '#') {
    serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::ChatMessage));
    std::stringstream ss;
    serialize(*buffer, 0);
    if (text == "#id") {
      ss << "id=" << player->getId();
    } else if (text == "#info") {
      float mass = 0;
      for (auto* item : m_foodContainer) {
        mass += item->mass;
      }
      for (auto* item : m_massContainer) {
        mass += item->mass;
      }
      for (auto* item : m_virusContainer) {
        mass += item->mass;
      }
      for (auto* item : m_phageContainer) {
        mass += item->mass;
      }
      for (auto* item : m_motherContainer) {
        mass += item->mass;
      }
      for (auto* item : m_avatarContainer) {
        mass += item->mass;
      }
      ss << "roomId=" << m_id << "; connections=" << m_sessions.size()
        << "; players=" << m_players.size()
        << "; totalMass=" << m_mass << ":" << mass
        << "; avatars=" << m_avatarContainer.size()
        << "; food=" << m_foodContainer.size()
        << "; masses=" << m_massContainer.size()
        << "; viruses=" << m_virusContainer.size()
        << "; phages=" << m_phageContainer.size()
        << "; mothers=" << m_motherContainer.size();
    }
    serialize(*buffer, ss.str());
  }
  send(buffer);
  m_chatHistory.emplace_front(player->getId(), player->name, text);
  while (m_chatHistory.size() > 128) {
    m_chatHistory.pop_back();
  }
}

void Room::interact(Avatar& avatar1, Avatar& avatar2)
{
  Avatar* attacker = &avatar1;
  Avatar* defender = &avatar2;
  if (attacker->mass < defender->mass) {
    std::swap(attacker, defender);
  }
  auto dd = geometry::squareDistance(attacker->position, defender->position);
  auto d = attacker->radius - 0.6 * defender->radius;
  if (attacker->player == defender->player) {
    if (!attacker->isRecombined() || !defender->isRecombined()) {
      return;
    }
    auto d2 = 0.25 * attacker->radius;
    if ((dd > d * d) && (dd > d2 * d2)) {
      return;
    }
    attacker->recombination(m_config.avatarRecombineTime);
  } else {
    if (attacker->mass < 1.25 * defender->mass || dd > d * d) {
      return;
    }
    attacker->player->wakeUp();
  }
  modifyMass(*attacker, defender->mass);
  m_zombieAvatars.push_back(defender);
  defender->zombie = true;
  Player& player = *defender->player;
  player.removeAvatar(defender);
  if (player.isDead()) {
    player.killer = attacker->player;
  }
  m_updateLeaderboard = true;
}

void Room::interact(Avatar& avatar, Food& food)
{
  auto distance = avatar.radius + food.radius;
  if (geometry::squareDistance(avatar.position, food.position) < distance * distance) {
    avatar.player->wakeUp();
    modifyMass(avatar, food.mass);
    m_zombieFoods.push_back(&food);
    food.zombie = true;
    m_updateLeaderboard = true;
  }
}

void Room::interact(Avatar& avatar, Mass& mass)
{
  if (avatar.mass < 1.25 * mass.mass) {
    return;
  }
  if (avatar.player == mass.player && mass.velocity) {
    return;
  }
  auto d = avatar.radius - 0.6 * mass.radius;
  if (geometry::squareDistance(avatar.position, mass.position) < d * d) {
    if (avatar.player != mass.player) {
      avatar.player->wakeUp();
    }
    modifyMass(avatar, mass.mass);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
    m_updateLeaderboard = true;
  }
}

void Room::interact(Avatar& avatar, Virus& virus)
{
  if (avatar.mass < 1.25 * virus.mass) {
    return;
  }
  auto distance = avatar.radius - 0.6 * virus.radius;
  if (geometry::squareDistance(avatar.position, virus.position) < distance * distance) {
    explode(avatar);
    m_zombieViruses.push_back(&virus);
    virus.zombie = true;
  }
}

void Room::interact(Avatar& avatar, Phage& phage)
{
  if (avatar.mass <= m_config.cellMinMass || avatar.mass < 1.25 * phage.mass) { // TODO: move the constant to the config
    return;
  }
  auto distance = avatar.radius + phage.radius;
  if (geometry::squareDistance(avatar.position, phage.position) < distance * distance) {
    auto m = std::min(static_cast<float>(m_simulationInterval * phage.mass), avatar.mass - m_config.cellMinMass);
    modifyMass(avatar, -m);
    m_modifiedCells.insert(&avatar);
  }
}

void Room::interact(Avatar& avatar, Mother& mother)
{
  auto dist = geometry::distance(avatar.position, mother.position);
  if (mother.mass >= 1.25 * avatar.mass && dist < mother.radius - 0.25 * avatar.radius) {
    modifyMass(mother, avatar.mass);
    m_activatedCells.insert(&mother);
    avatar.player->removeAvatar(&avatar);
    m_zombieAvatars.push_back(&avatar);
    avatar.zombie = true;
    m_updateLeaderboard = true;
  } else if (avatar.mass > 1.25 * mother.mass && dist < avatar.radius - 0.25 * mother.radius) {
    explode(avatar);
    m_zombieMothers.push_back(&mother);
    mother.zombie = true;
  }
}

void Room::interact(Mother& mother, Food& food)
{
  if (food.creator == &mother && food.velocity) {
    return;
  }
  auto d = mother.radius;
  if (geometry::squareDistance(mother.position, food.position) < d * d) {
    m_zombieFoods.push_back(&food);
    food.zombie = true;
  }
}

void Room::interact(Mother& mother, Mass& mass)
{
  auto distance = mother.radius - 0.25 * mass.radius;
  if (geometry::squareDistance(mother.position, mass.position) < distance * distance) {
    modifyMass(mother, mass.mass);
    m_activatedCells.insert(&mother);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Mother& mother, Virus& virus)
{
  auto distance = mother.radius + virus.radius;
  if (geometry::squareDistance(mother.position, virus.position) < distance * distance) {
    modifyMass(mother, virus.mass);
    m_activatedCells.insert(&mother);
    m_zombieViruses.push_back(&virus);
    virus.zombie = true;
  }
}

void Room::interact(Mother& mother, Phage& phage)
{
  auto distance = mother.radius + phage.radius;
  if (geometry::squareDistance(mother.position, phage.position) < distance * distance) {
    modifyMass(mother, phage.mass);
    m_activatedCells.insert(&mother);
    m_zombiePhages.push_back(&phage);
    phage.zombie = true;
  }
}

void Room::interact(Virus& virus, Food& food)
{
  auto distance = virus.radius + food.radius;
  if (geometry::squareDistance(virus.position, food.position) < distance * distance) {
    m_zombieFoods.push_back(&food);
    food.zombie = true;
  }
}

void Room::interact(Virus& virus, Mass& mass)
{
  auto distance = virus.radius + mass.radius;
  if (geometry::squareDistance(virus.position, mass.position) < distance * distance) {
    auto relativeVelocity = virus.velocity - mass.velocity;
    auto normal = (virus.position - mass.position).direction();
    auto impulse = relativeVelocity * normal * (2.0f * virus.mass * mass.mass / (virus.mass + mass.mass));
    virus.velocity -= normal * impulse / virus.mass;
    m_modifiedCells.insert(&virus);
    m_activatedCells.insert(&virus);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Virus& virus1, Virus& virus2)
{
  auto distance = virus1.radius + virus2.radius;
  if (geometry::squareDistance(virus1.position, virus2.position) < distance * distance) {
    auto& obj = createVirus();
    modifyMass(obj, virus1.mass + virus2.mass);
    obj.position = (virus1.position + virus2.position) * 0.5;
    m_zombieViruses.push_back(&virus1);
    m_zombieViruses.push_back(&virus2);
    virus1.zombie = true;
    virus2.zombie = true;
  }
}

void Room::interact(Virus& virus, Phage& phage)
{
  auto distance = virus.radius + phage.radius;
  if (geometry::squareDistance(virus.position, phage.position) < distance * distance) {
    auto& obj = createMother();
    modifyMass(obj, virus.mass + phage.mass);
    obj.position = (virus.position + phage.position) * 0.5;
    m_zombieViruses.push_back(&virus);
    m_zombiePhages.push_back(&phage);
    virus.zombie = true;
    phage.zombie = true;
  }
}

void Room::interact(Phage& phage, Food& food)
{
  auto distance = phage.radius + food.radius;
  if (geometry::squareDistance(phage.position, food.position) < distance * distance) {
    m_zombieFoods.push_back(&food);
    food.zombie = true;
  }
}

void Room::interact(Phage& phage, Mass& mass)
{
  auto distance = phage.radius + mass.radius;
  if (geometry::squareDistance(phage.position, mass.position) < distance * distance) {
    auto relativeVelocity = phage.velocity - mass.velocity;
    auto normal = (phage.position - mass.position).direction();
    auto impulse = relativeVelocity * normal * (2.0f * phage.mass * mass.mass / (phage.mass + mass.mass));
    phage.velocity -= normal * impulse / phage.mass;
    m_modifiedCells.insert(&phage);
    m_activatedCells.insert(&phage);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Phage& phage1, Phage& phage2)
{
  auto distance = phage1.radius + phage2.radius;
  if (geometry::squareDistance(phage1.position, phage2.position) < distance * distance) {
    auto& obj = createPhage();
    modifyMass(obj, phage1.mass + phage2.mass);
    obj.position = (phage1.position + phage2.position) * 0.5;
    m_zombiePhages.push_back(&phage1);
    m_zombiePhages.push_back(&phage2);
    phage1.zombie = true;
    phage2.zombie = true;
  }
}

void Room::attract(Avatar& initiator, Avatar& target)
{
  if (initiator.player == target.player) {
    return;
  }
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((target.position - initiator.position).direction() * initiator.maxSpeed);
  float forceRatio = 0;
  float ratio = initiator.mass / target.mass;
  if (ratio >= 1.25) {
    forceRatio = m_config.botForceHungerRatio * ratio;
  } else if (ratio <= 0.8) {
    velocity *= -1;
    forceRatio = m_config.botForceDangerRatio;
  } else {
    return;
  }
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * forceRatio / dist);
}

void Room::attract(Avatar& initiator, Food& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((target.position - initiator.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceFoodRatio / dist);
}

void Room::attract(Avatar& initiator, Mass& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((target.position - initiator.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceHungerRatio / dist);
}

void Room::attract(Avatar& initiator, Virus& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::attract(Avatar& initiator, Phage& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::attract(Avatar& initiator, Mother& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::attract(Avatar& initiator, const Vec2D& point)
{
  float dist(geometry::squareDistance(point, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - point).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * (initiator.mass * m_config.botForceCornerRatio / dist);
}

void Room::integrate(Avatar& initiator, const Vec2D& point)
{
  float dist(geometry::squareDistance(point, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - point).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * (initiator.mass * m_config.botForceCornerRatio / dist);
}

void Room::update()
{
  auto now{TimePoint::clock::now()};
  auto deltaTime{now - m_lastUpdate};
  m_lastUpdate = now;
  ++m_tick;
  auto dt = duration_cast<duration<double>>(deltaTime).count(); // TODO: replace to deltaTime

  generate(dt);
  handlePlayerRequests();

  for (auto i = m_config.simulationsPerUpdate; i > 0; --i) {
    simulate(dt / m_config.simulationsPerUpdate);
  }

  auto deflationTime = now - m_config.playerDeflationInterval;
  auto annihilationTime = now - m_config.playerAnnihilationInterval;
  for (auto* avatar : m_avatarContainer) {
    const auto& lastActivity = avatar->player->getLastActivity();
    if (lastActivity < deflationTime) {
      float mass = avatar->mass * m_config.playerDeflationRatio * dt;
      if (avatar->mass - mass >= m_config.cellMinMass) {
        modifyMass(*avatar, -mass);
        m_updateLeaderboard = true;
      }
    }
    if (lastActivity < annihilationTime) {
      avatar->player->removeAvatar(avatar);
      m_zombieAvatars.push_back(avatar);
      avatar->zombie = true;
      m_updateLeaderboard = true;
      if (m_mass < m_config.maxMass) {
        auto& obj = createVirus();
        modifyMass(obj, avatar->mass > m_config.virusStartMass ? avatar->mass : m_config.virusStartMass);
        obj.position = avatar->position;
        obj.color = avatar->color;
      }
    }
  }

  // TODO: optimize amount of containers
  for (Avatar* avatar : m_zombieAvatars) {
    m_avatarContainer.erase(avatar);
    removeCell(avatar);
  }
  m_zombieAvatars.clear();
  for (Food* food : m_zombieFoods) {
    m_foodContainer.erase(food);
    removeCell(food);
  }
  m_zombieFoods.clear();
  for (Mass* mass : m_zombieMasses) {
    m_massContainer.erase(mass);
    removeCell(mass);
  }
  m_zombieMasses.clear();
  for (Virus* virus : m_zombieViruses) {
    m_virusContainer.erase(virus);
    removeCell(virus);
  }
  m_zombieViruses.clear();
  for (Phage* phage : m_zombiePhages) {
    m_phageContainer.erase(phage);
    removeCell(phage);
  }
  m_zombiePhages.clear();
  for (Mother* mother : m_zombieMothers) {
    m_motherContainer.erase(mother);
    removeCell(mother);
  }
  m_zombieMothers.clear();

  std::vector<Mother*> mothers;
  mothers.reserve(m_motherContainer.size());
  for (auto* mother : m_motherContainer) {
    if (mother->mass >= m_config.motherExplodeMass) {
      mothers.emplace_back(mother);
    }
  }
  for (Mother* mother : mothers) {
    explode(*mother);
  }

  synchronize();
}

Vec2D Room::getRandomPosition(uint32_t radius) const
{
  Circle c;
  c.radius = radius;
  for (uint32_t n = m_config.spawnPosTryCount; n > 0; --n) {
    bool intersect = false;
    c.position.x = (m_generator() % (m_config.width - 2 * radius)) + radius;
    c.position.y = (m_generator() % (m_config.height - 2 * radius)) + radius;
    for (const auto& cell : m_forCheckRandomPos) {
      if (geometry::intersects(*cell, c)) {
        intersect = true;
        break;
      }
    }
    if (!intersect) {
      break;
    }
  }
  return c.position;
}

Avatar& Room::createAvatar()
{
  auto* cell = new Avatar(*this, m_cellNextId.pop());
  m_avatarContainer.insert(cell);
  updateNewCellRegistries(cell);
  return *cell;
}

Food& Room::createFood()
{
  auto* cell = new Food(*this, m_cellNextId.pop());
  m_foodContainer.insert(cell);
  updateNewCellRegistries(cell, false);
  return *cell;
}

Mass& Room::createMass()
{
  auto* cell = new Mass(*this, m_cellNextId.pop());
  m_massContainer.insert(cell);
  updateNewCellRegistries(cell, false);
  return *cell;
}

Virus& Room::createVirus()
{
  auto* cell = new Virus(*this, m_cellNextId.pop());
  m_virusContainer.insert(cell);
  updateNewCellRegistries(cell);
  return *cell;
}

Phage& Room::createPhage()
{
  auto* cell = new Phage(*this, m_cellNextId.pop());
  m_phageContainer.insert(cell);
  updateNewCellRegistries(cell);
  return *cell;
}

Mother& Room::createMother()
{
  auto* cell = new Mother(*this, m_cellNextId.pop());
  m_motherContainer.insert(cell);
  updateNewCellRegistries(cell);
  return *cell;
}

void Room::recalculateFreeSpace()
{
  m_hasFreeSpace = m_players.size() < m_config.maxPlayers;
}

void Room::updateNewCellRegistries(Cell* cell, bool checkRandomPos)
{
  m_createdCells.push_back(cell);
  m_modifiedCells.insert(cell);
  m_activatedCells.insert(cell);
  if (checkRandomPos) {
    m_forCheckRandomPos.insert(cell);
  }
}

void Room::removeCell(Cell* cell)
{
  m_mass -= cell->mass;
  m_forCheckRandomPos.erase(cell);
  m_processingCells.erase(cell);
  m_modifiedCells.erase(cell);
  m_activatedCells.erase(cell);
  m_gridmap.erase(cell);
  m_removedCellIds.push_back(cell->id);
  m_cellNextId.push(cell->id);
  delete cell;
}

bool Room::eject(Avatar& avatar, const Vec2D& point)
{
  float massLoss = m_config.avatarEjectMassLoss;
  if (avatar.mass < m_config.avatarEjectMinMass || avatar.mass - massLoss < m_config.cellMinMass) {
    return false;
  }
  modifyMass(avatar, -massLoss);
  auto& obj = createMass();
  obj.player = avatar.player;
  obj.creator = &avatar;
  obj.color = avatar.color;
  obj.position = avatar.position;
  obj.velocity = avatar.velocity;
  modifyMass(obj, m_config.avatarEjectMass);
  auto direction = point - avatar.position;
  if (direction) {
    direction.normalize();
    obj.applyVelocity(direction * m_config.avatarEjectVelocity);
    obj.position += direction * avatar.radius;
  }
  m_updateLeaderboard = true;
  return true;
}

bool Room::split(Avatar& avatar, const Vec2D& point)
{
  float mass = 0.5 * avatar.mass;
  if (avatar.mass < m_config.avatarSplitMinMass || mass < m_config.cellMinMass) {
    return false;
  }
  auto& obj = createAvatar();
  obj.color = avatar.color;
  obj.position = avatar.position;
  obj.velocity = avatar.velocity;
  obj.protection = m_tick + 40;
  modifyMass(obj, mass);
  modifyMass(avatar, -mass);
  auto direction = point - avatar.position;
  if (direction) {
    direction.normalize();
    obj.applyVelocity(direction * m_config.avatarSplitVelocity);
  }
  obj.recombination(m_config.avatarRecombineTime);
  avatar.recombination(m_config.avatarRecombineTime);
  avatar.player->addAvatar(&obj);
  return true;
}

void Room::explode(Avatar& avatar)
{
  const auto& avatars = avatar.player->getAvatars();
  if (avatars.size() >= m_config.playerMaxCells) {
    return;
  }
  float explodedMass = 0;
  float minMass = std::max(m_config.avatarExplodeMinMass, m_config.cellMinMass);
  for (auto n = m_config.avatarExplodeParts; n > 0; --n) {
    float mass = 0.125 * avatar.mass;
    if (mass < minMass) {
      mass = minMass;
    }
    if (explodedMass + mass > avatar.mass) {
      break;
    }
    explodedMass += mass;
    auto& obj = createAvatar();
    modifyMass(obj, mass);
    float angle = (m_generator() % 3600) * M_PI / 1800;
    Vec2D direction(sin(angle), cos(angle));
    obj.position = avatar.position + direction * (avatar.radius + obj.radius);
    obj.color = avatar.color;
    obj.applyVelocity(direction * m_config.explodeVelocity);
    obj.recombination(m_config.avatarRecombineTime);
    avatar.player->addAvatar(&obj);
    if (avatars.size() >= m_config.playerMaxCells) {
      break;
    }
  }
  if (explodedMass) {
    avatar.recombination(m_config.avatarRecombineTime);
    modifyMass(avatar, -explodedMass);
  }
}

void Room::explode(Mother& mother)
{
  float doubleMass = m_config.motherStartMass * 2;
  while (mother.mass >= doubleMass) {
    auto& obj = createMother();
    modifyMass(obj, m_config.motherStartMass);
    float angle = (m_generator() % 3600) * M_PI / 1800;
    Vec2D direction(sin(angle), cos(angle));
    obj.position = mother.position;
    obj.applyVelocity(direction * m_config.explodeVelocity);
    modifyMass(mother, -static_cast<float>(m_config.motherStartMass));
  }
}

void Room::modifyMass(Cell& cell, float value)
{
  cell.mass += value;
  m_mass += value;
  if (cell.mass < m_config.cellMinMass) {
    spdlog::warn("Bad cell mass. type={}, mass={}, value={}", cell.type, cell.mass, value);
    cell.mass = m_config.cellMinMass;
  }
  cell.radius = m_config.cellRadiusRatio * sqrt(cell.mass / M_PI);
  m_modifiedCells.insert(&cell);
}

void Room::modifyMass(Avatar& avatar, float value)
{
  modifyMass(static_cast<Cell&>(avatar), value);
  avatar.maxSpeed = m_config.avatarMaxSpeed - m_avatarSpeedDiff * (avatar.radius - m_cellMinRadius) / (m_cellRadiusDiff);
}

void Room::solveCellLocation(Cell& cell)
{
  AABB box;
  box.b.x = m_config.width;
  box.b.y = m_config.height;
  auto delta = box.a.x - cell.position.x + cell.radius;
  if (delta >= 0) {
    cell.position.x += delta;
    cell.velocity.x = -cell.velocity.x;
    m_modifiedCells.insert(&cell);
  } else {
    delta = cell.position.x + cell.radius - box.b.x;
    if (delta >= 0) {
      cell.position.x -= delta;
      cell.velocity.x = -cell.velocity.x;
      m_modifiedCells.insert(&cell);
    }
  }
  delta = box.a.y - cell.position.y + cell.radius;
  if (delta >= 0) {
    cell.position.y += delta;
    cell.velocity.y = -cell.velocity.y;
    m_modifiedCells.insert(&cell);
  } else {
    delta = cell.position.y + cell.radius - box.b.x;
    if (delta >= 0) {
      cell.position.y -= delta;
      cell.velocity.y = -cell.velocity.y;
      m_modifiedCells.insert(&cell);
    }
  }
}

void Room::destroyOutdatedCells()
{
  auto currentTime = TimePoint::clock::now();
  TimePoint expirationTime;

  auto expired = [&](Cell* cell)
    {
      if (cell->created < expirationTime) {
        removeCell(cell);
        return true;
      };
      return false;
    };

  expirationTime = currentTime - m_config.virusLifeTime;
  std::erase_if(m_virusContainer, expired);

  expirationTime = currentTime - m_config.phageLifeTime;
  std::erase_if(m_phageContainer, expired);

  expirationTime = currentTime - m_config.motherLifeTime;
  std::erase_if(m_motherContainer, expired);
}

void Room::generate(float dt)
{
  if (m_mass >= m_config.maxMass) {
    return;
  }

  auto avail = m_config.foodMaxAmount - m_foodContainer.size();
  if (avail > 0 && m_mass + m_accumulatedFoodMass < m_config.maxMass) {
    m_accumulatedFoodMass += m_config.spawnFoodMass * dt;
    auto count = std::min(static_cast<size_t>(m_accumulatedFoodMass / m_config.foodMass), avail);
    if (count > 0) {
      m_accumulatedFoodMass -= m_config.foodMass * count;
      spawnFood(count);
    }
  }

  avail = m_config.virusMaxAmount - m_virusContainer.size();
  if (avail > 0 && m_mass + m_accumulatedVirusMass < m_config.maxMass) {
    m_accumulatedVirusMass += m_config.spawnVirusMass * dt;
    auto count = std::min(static_cast<size_t>(m_accumulatedVirusMass / m_config.virusStartMass), avail);
    if (count > 0) {
      m_accumulatedVirusMass -= m_config.virusStartMass * count;
      spawnViruses(count);
    }
  }

  avail = m_config.phageMaxAmount - m_phageContainer.size();
  if (avail > 0 && m_mass + m_accumulatedPhageMass < m_config.maxMass) {
    m_accumulatedPhageMass += m_config.spawnPhageMass * dt;
    auto count = std::min(static_cast<size_t>(m_accumulatedPhageMass / m_config.phageStartMass), avail);
    if (count > 0) {
      m_accumulatedPhageMass -= m_config.phageStartMass * count;
      spawnPhages(count);
    }
  }

  avail = m_config.motherMaxAmount - m_motherContainer.size();
  if (avail > 0 && m_mass + m_accumulatedMotherMass < m_config.maxMass) {
    m_accumulatedMotherMass += m_config.spawnMotherMass * dt;
    auto count = std::min(static_cast<size_t>(m_accumulatedMotherMass / m_config.motherStartMass), avail);
    if (count > 0) {
      m_accumulatedMotherMass -= m_config.motherStartMass * count;
      spawnMothers(count);
    }
  }
}

void Room::checkMothers()
{
  for (auto* mother : m_motherContainer) {
    Vec2D radius(mother->radius + m_config.motherCheckRadius, mother->radius + m_config.motherCheckRadius);
    AABB box(mother->position - radius, mother->position + radius);
    mother->foodCount = m_gridmap.count(box);
  }
}

void Room::handlePlayerRequests()
{
  for (const auto& it : m_pointerRequests) {
    const auto& sess = it.first;
    auto* player = sess->player();
    if (player && !player->isDead()) {
      player->setPointer(it.second);
      const auto& avatars = player->getAvatars();
      m_modifiedCells.insert(avatars.begin(), avatars.end());
      m_processingCells.insert(avatars.begin(), avatars.end());
    }
  }
  m_pointerRequests.clear();

  for (const auto& it : m_ejectRequests) {
    const auto& sess = it.first;
    auto* player = sess->player();
    if (player && !player->isDead()) {
      for (auto* avatar : player->getAvatars()) {
        eject(*avatar, it.second);
      }
    }
  }
  m_ejectRequests.clear();

  for (const auto& it : m_splitRequests) {
    const auto& sess = it.first;
    auto* player = sess->player();
    if (player && !player->isDead()) {
      auto avatars = player->getAvatars();
      size_t count = avatars.size();
      if (count >= m_config.playerMaxCells) {
        break;
      }
      for (Avatar* avatar : avatars) {
        if (split(*avatar, it.second)) {
          ++count;
          if (count >= m_config.playerMaxCells) {
            break;
          }
        }
      }
    }
  }
  m_splitRequests.clear();
}

void Room::simulate(float dt)
{
  for (auto* bot : m_bots) {
    bot->choseTarget();
  }
  for (auto* player : m_fighters) {
    player->applyDestinationAttractionForce(m_tick);
    player->recombine(m_tick);
  }
  for (auto* cell : m_processingCells) {
    cell->applyResistanceForce();
    if (cell->force) {
      m_modifiedCells.insert(cell);
    }
    cell->simulate(dt);
    solveCellLocation(*cell);
    m_gridmap.update(cell);
  }
  for (auto* cell : m_processingCells) {
    if (!cell->zombie) {
      m_gridmap.query(cell->getAABB(), [cell](Cell& target) -> bool {
        if (cell != &target && !cell->zombie && !target.zombie) {
          cell->interact(target);
        }
        return !cell->zombie;
      });
    }
  }
}

void Room::synchronize()
{
  std::erase_if(m_processingCells,
    [this](Cell* cell)
    {
      if (!cell->velocity) {
        m_modifiedCells.insert(cell);
        return true;
      }
      return false;
    }
  );
  if (!m_activatedCells.empty()) {
    m_processingCells.insert(m_activatedCells.begin(), m_activatedCells.end());
    m_activatedCells.clear();
  }

  Player* topPlayer = nullptr;
  for (Player* player : m_leaderboard) {
    if (!player->isDead()) {
      topPlayer = player;
      break;
    }
  }

  auto last = m_leaderboard.end();
  for (Player* player : m_fighters) {
    player->calcParams();
    player->synchronize(m_tick, m_modifiedCells, m_removedCellIds);
    if (player->isDead()) {
      last = std::remove(m_leaderboard.begin(), last, player);
      m_updateLeaderboard = true;
      sendPacketPlayerDead(player->getId());
      Player* observable = player->killer;
      if (!observable || observable->isDead()) {
        observable = topPlayer;
      }
      const auto& buffer = std::make_shared<Buffer>();
      if (observable) {
        PacketSpectate packet(*observable);
        packet.format(*buffer);
        for (const auto& sess : player->getSessions()) {
          sess->send(buffer);
          observable->addSession(sess);
          sess->observable(observable);
        }
      } else {
        EmptyPacket packet(OutputPacketTypes::Finish);
        packet.format(*buffer);
        for (const auto& sess : player->getSessions()) {
          sess->send(buffer);
        }
      }
      player->clearSessions();
    }
  }
  m_leaderboard.erase(last, m_leaderboard.end());
  m_modifiedCells.clear();
  m_removedCellIds.clear();
  if (!m_createdCells.empty()) {
    for (Cell* cell : m_createdCells) {
      m_gridmap.insert(cell);
      cell->newly = false;
    }
    m_createdCells.clear();
  }

  std::erase_if(m_fighters, [&](auto* player) { return player->isDead(); });
}

void Room::updateLeaderboard()
{
  if (m_updateLeaderboard) {
    std::sort(m_leaderboard.begin(), m_leaderboard.end(), [] (Player* a, Player* b) { return *b < *a; });
    const auto& buffer = std::make_shared<Buffer>();
    PacketLeaderboard packetLeaderboard{m_leaderboard, m_config.leaderboardVisibleItems};
    packetLeaderboard.format(*buffer);
    send(buffer);
    m_updateLeaderboard = false;
  }
}

void Room::produceMothers()
{
  if (m_motherContainer.empty() || m_mass >= m_config.maxMass || m_foodContainer.size() >= m_config.foodMaxAmount) {
    return;
  }

  for (auto* mother : m_motherContainer) {
    if (mother->foodCount >= 100) {
      continue;
    }
    int cnt = 0;
    int bonus = mother->mass - m_config.motherStartMass;
    if (bonus > 0) {
      cnt = bonus > 100 ? 20 : bonus > 25 ? 5 : 1;
      modifyMass(*mother, -cnt);
    } else {
      cnt = mother->foodCount < 20 ? 5 : 1;
    }
    mother->foodCount += cnt;
    uint32_t impulse = m_config.foodMaxVelocity - m_config.foodMinVelocity;
    for (int i=0; i<cnt; ++i) {
      auto angle = M_PI * (m_generator() % 3600) / 1800;
      Vec2D direction(sin(angle), cos(angle));
      auto& obj = createFood();
      obj.creator = mother;
      obj.position = mother->position;
      if (mother->radius > mother->startRadius) {
        obj.position += direction * (mother->radius - mother->startRadius);
      }
      obj.color = m_generator() % 16;
      obj.mass = m_config.foodMass;
      obj.radius = m_config.foodRadius;
      float magnitude = m_config.foodMinVelocity + (m_generator() % impulse);
      obj.applyVelocity(direction * magnitude);
      m_mass += obj.mass;
    }
  }
}

void Room::checkPlayers()
{
  for (auto* bot : m_bots) {
    if (bot->isDead()) {
      spawnBot(bot->getId());
    }
  }

  auto expirationTime = TimePoint::clock::now() - m_config.playerAnnihilationInterval;
  std::erase_if(m_zombiePlayers,
    [&](auto* player)
    {
      if (player->isDead() && player->getLastActivity() < expirationTime) {
        m_players.erase(player->getId());
        for (auto& it : m_players) {
          it.second->removePlayer(player);
        }
        return true;
      }
      return false;
    }
  );

  recalculateFreeSpace();
}

void Room::spawnFood(uint32_t count)
{
  for (; count>0; --count) {
    auto& obj = createFood();
    obj.position = getRandomPosition(m_config.foodRadius);
    obj.color = m_generator() % 16;
    obj.mass = m_config.foodMass;
    obj.radius = m_config.foodRadius;
    m_mass += obj.mass;
  }
}

void Room::spawnViruses(uint32_t count)
{
  for (; count>0; --count) {
    auto& obj = createVirus();
    modifyMass(obj, m_config.virusStartMass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::spawnPhages(uint32_t count)
{
  for (; count>0; --count) {
    auto& obj = createPhage();
    modifyMass(obj, m_config.phageStartMass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::spawnMothers(uint32_t count)
{
  for (; count>0; --count) {
    auto& obj = createMother();
    modifyMass(obj, m_config.motherStartMass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::spawnBot(uint32_t id, const std::string& name)
{
  const auto& it = m_players.find(id);
  Bot* bot;
  if (it != m_players.end()) {
    bot = dynamic_cast<Bot*>(it->second);
  } else {
    bot = new Bot(id, *this, m_gridmap);
    bot->name = name.empty() ? "Bot " + std::to_string(id) : name;
    bot->online = true;
    m_players.emplace(bot->getId(), bot);
    recalculateFreeSpace();
    m_bots.emplace(bot);
    sendPacketPlayer(*bot);
  }
  if (bot->isDead()) {
    auto& obj = createAvatar();
    modifyMass(obj, m_config.botStartMass);
    int radius = obj.radius;
    obj.position.x = (m_generator() % (m_config.width - 2 * radius)) + radius;
    obj.position.y = (m_generator() % (m_config.height - 2 * radius)) + radius;
    obj.color = m_generator() % 12;
    bot->addAvatar(&obj);
    bot->calcParams();
    bot->wakeUp();
    m_leaderboard.emplace_back(bot);
    m_updateLeaderboard = true;
    m_fighters.insert(bot);
    sendPacketPlayerBorn(bot->getId());
  }
  bot->init();
}

void Room::send(const BufferPtr& buffer)
{
  for (const auto& sess : m_sessions) {
    sess->send(buffer);
  }
}

void Room::sendPacketPlayer(const Player& player)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::Player));
  serialize(*buffer, player.getId());
  serialize(*buffer, player.name);
  send(buffer);
}

void Room::sendPacketPlayerRemove(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::PlayerRemove));
  serialize(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerJoin(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::PlayerJoin));
  serialize(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerLeave(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::PlayerLeave));
  serialize(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerBorn(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::PlayerBorn));
  serialize(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerDead(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::PlayerDead));
  serialize(*buffer, playerId);
  send(buffer);
}
