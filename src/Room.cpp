// file   : src/Room.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Room.hpp"

#include "OutgoingPacket.hpp"
#include "Session.hpp"
#include "Player.hpp"
#include "Bot.hpp"
#include "User.hpp"

#include "entity/Avatar.hpp"
#include "entity/Food.hpp"
#include "entity/Bullet.hpp"
#include "entity/Virus.hpp"
#include "entity/Phage.hpp"
#include "entity/Mother.hpp"

#include "geometry/geometry.hpp"
#include "serialization.hpp"

#include <boost/asio/post.hpp>

#include <spdlog/spdlog.h>
#include <chrono>

using namespace std::placeholders;

Room::Room(asio::any_io_executor executor, uint32_t id)
  : m_executor(std::move(executor))
  , m_updateTimer(m_executor, std::bind_front(&Room::update, this))
  , m_checkPlayersTimer(m_executor, std::bind_front(&Room::checkPlayers, this))
  , m_updateLeaderboardTimer(m_executor, std::bind_front(&Room::updateLeaderboard, this))
  , m_destroyOutdatedCellsTimer(m_executor, std::bind_front(&Room::destroyOutdatedCells, this))
  , m_checkMothersTimer(m_executor, std::bind_front(&Room::checkMothers, this))
  , m_produceMothersTimer(m_executor, std::bind_front(&Room::produceMothers, this))
  , m_foodGeneratorTimer(m_executor, std::bind_front(&Room::generateFood, this))
  , m_virusGeneratorTimer(m_executor, std::bind_front(&Room::generateViruses, this))
  , m_phageGeneratorTimer(m_executor, std::bind_front(&Room::generatePhages, this))
  , m_motherGeneratorTimer(m_executor, std::bind_front(&Room::generateMothers, this))
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
  for (auto* it : m_bulletContainer) {
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

const asio::any_io_executor& Room::getExecutor() const
{
  return m_executor;
}

void Room::init(const config::Room& config)
{
  m_config = config;

  m_updateTimer.setInterval(m_config.updateInterval);
  m_checkPlayersTimer.setInterval(m_config.checkPlayersInterval);
  m_updateLeaderboardTimer.setInterval(m_config.leaderboard.updateInterval);
  m_destroyOutdatedCellsTimer.setInterval(m_config.destroyOutdatedCellsInterval);
  m_checkMothersTimer.setInterval(m_config.checkMothersInterval);
  m_produceMothersTimer.setInterval(m_config.produceMothersInterval);
  m_foodGeneratorTimer.setInterval(m_config.generator.food.interval);
  m_virusGeneratorTimer.setInterval(m_config.generator.virus.interval);
  m_phageGeneratorTimer.setInterval(m_config.generator.phage.interval);
  m_motherGeneratorTimer.setInterval(m_config.generator.mother.interval);

  m_gridmap.resize(m_config.width, m_config.height, 9);

  spawnFood(m_config.food.quantity);
  spawnViruses(m_config.virus.quantity);
  spawnPhages(m_config.phage.quantity);
  spawnMothers(m_config.mother.quantity);

  createBots();
}

void Room::start()
{
  m_updateTimer.start();
  m_checkPlayersTimer.start();
  m_updateLeaderboardTimer.start();
  m_destroyOutdatedCellsTimer.start();
  m_checkMothersTimer.start();
  m_produceMothersTimer.start();
  if (m_config.generator.food.enabled) {
    m_foodGeneratorTimer.start();
  }
  if (m_config.generator.virus.enabled) {
    m_virusGeneratorTimer.start();
  }
  if (m_config.generator.phage.enabled) {
    m_phageGeneratorTimer.start();
  }
  if (m_config.generator.mother.enabled) {
    m_motherGeneratorTimer.start();
  }
}

void Room::stop()
{
  m_updateTimer.stop();
  m_checkPlayersTimer.stop();
  m_updateLeaderboardTimer.stop();
  m_destroyOutdatedCellsTimer.stop();
  m_checkMothersTimer.stop();
  m_produceMothersTimer.stop();
  m_foodGeneratorTimer.stop();
  m_virusGeneratorTimer.stop();
  m_phageGeneratorTimer.stop();
  m_motherGeneratorTimer.stop();
  for (auto* bot : m_bots) {
    bot->stop();
  }
}

uint32_t Room::getId() const
{
  return m_id;
}

bool Room::hasFreeSpace() const
{
  return m_hasFreeSpace;
}

const config::Room& Room::getConfig() const
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

void Room::move(const SessionPtr& sess, const Vec2D& point)
{
  asio::post(m_executor, std::bind_front(&Room::doMove, this, sess, point));
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
  serialize(*buffer);

  // TODO: revise
  uint32_t playerId = user->getId();
  const auto& it = m_players.find(playerId);
  if (it != m_players.end()) {
    it->second->setMainSession(sess);
  } else {
    OutgoingPacket::serializeFinish(*buffer);
    playerId = 0;
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
    m_moveRequests.erase(sess);
    m_ejectRequests.erase(sess);
    m_splitRequests.erase(sess);
    if (auto* player = sess->player()) {
      sendPacketPlayerLeave(player->getId());
      player->removeSession(sess);
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

  auto* player = sess->player();
  if (!player) {
    uint32_t playerId = user->getId();
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = createPlayer(playerId, name);
      sendPacketPlayer(*player);
    }
    player->setMainSession(sess);
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
  if (player->getName() != name) { // TODO: encapsulate logic in Player::setName
    player->setName(name);
    sendPacketPlayer(*player);
  }
  sendPacketPlayerBorn(player->getId());

  auto& obj = createAvatar();
  obj.modifyMass(m_config.player.mass);
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
  OutgoingPacket::serializePlay(*buffer, *player);
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
      player = new Player(m_executor, playerId, *this, m_gridmap);
      player->setName("Player " + std::to_string(user->getId()));
      m_players.emplace(playerId, player);
      recalculateFreeSpace();
      sendPacketPlayer(*player);
    }
    player->addSession(sess);
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
  OutgoingPacket::serializeSpectate(*buffer, *target);
  sess->send(buffer);
  target->addSession(sess);
  sess->observable(target);
}

void Room::doMove(const SessionPtr& sess, const Vec2D& point)
{
  m_moveRequests.emplace(sess, point);
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
    player->setTargetPlayer(it->second);
  }
}

void Room::doChatMessage(const SessionPtr& sess, const std::string& text)
{
  auto* player = sess->player();
  if (!player){
    return;
  }
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializeChatMessage(*buffer, player->getId(), text);
  send(buffer);
  m_chatHistory.emplace_front(player->getId(), player->getName(), text);
  while (m_chatHistory.size() > 128) {
    m_chatHistory.pop_back();
  }
}

Avatar& Room::createAvatar()
{
  auto* avatar = new Avatar(*this, m_cellNextId.pop());
  avatar->subscribeToDeathEvent(this, std::bind(&Room::onAvatarDeath, this, avatar));
  avatar->subscribeToMassChangeEvent(this, std::bind(&Room::onAvatarMassChange, this, avatar, _1));
  avatar->subscribeToMotionStartedEvent(this, std::bind(&Room::onMotionStarted, this, avatar));
  m_avatarContainer.insert(avatar);
  updateNewCellRegistries(avatar);
  return *avatar;
}

Food& Room::createFood()
{
  auto* food = new Food(*this, m_cellNextId.pop());
  food->subscribeToDeathEvent(this, std::bind(&Room::onFoodDeath, this, food));
  m_foodContainer.insert(food);
  updateNewCellRegistries(food, false);
  return *food;
}

Bullet& Room::createBullet()
{
  auto* bullet = new Bullet(*this, m_cellNextId.pop());
  bullet->subscribeToDeathEvent(this, std::bind(&Room::onBulletDeath, this, bullet));
  m_bulletContainer.insert(bullet);
  updateNewCellRegistries(bullet, false);
  return *bullet;
}

Virus& Room::createVirus()
{
  auto* virus = new Virus(*this, m_cellNextId.pop());
  virus->subscribeToDeathEvent(this, std::bind(&Room::onVirusDeath, this, virus));
  virus->subscribeToMassChangeEvent(this, std::bind(&Room::onCellMassChange, this, virus, _1));
  virus->subscribeToMotionStartedEvent(this, std::bind(&Room::onMotionStarted, this, virus));
  m_virusContainer.insert(virus);
  updateNewCellRegistries(virus);
  return *virus;
}

Phage& Room::createPhage()
{
  auto* phage = new Phage(*this, m_cellNextId.pop());
  phage->subscribeToDeathEvent(this, std::bind(&Room::onPhageDeath, this, phage));
  phage->subscribeToMassChangeEvent(this, std::bind(&Room::onCellMassChange, this, phage, _1));
  phage->subscribeToMotionStartedEvent(this, std::bind(&Room::onMotionStarted, this, phage));
  m_phageContainer.insert(phage);
  updateNewCellRegistries(phage);
  return *phage;
}

Mother& Room::createMother()
{
  auto* mother = new Mother(*this, m_cellNextId.pop());
  mother->subscribeToDeathEvent(this, std::bind(&Room::onMotherDeath, this, mother));
  mother->subscribeToMassChangeEvent(this, std::bind(&Room::onMotherMassChange, this, mother, _1));
  m_motherContainer.insert(mother);
  updateNewCellRegistries(mother);
  return *mother;
}

void Room::explode(Avatar& avatar)
{
  const auto& avatars = avatar.player->getAvatars();
  if (avatars.size() >= m_config.player.maxCells) {
    return;
  }
  float explodedMass = 0;
  float minMass = std::max(m_config.avatar.explosionMinMass, m_config.cellMinMass);
  for (auto n = m_config.avatar.explosionParts; n > 0; --n) {
    float mass = 0.125 * avatar.mass;
    if (mass < minMass) {
      mass = minMass;
    }
    if (explodedMass + mass > avatar.mass) {
      break;
    }
    explodedMass += mass;
    auto& obj = createAvatar();
    obj.modifyMass(mass);
    float angle = (m_generator() % 3600) * M_PI / 1800;
    Vec2D direction(sin(angle), cos(angle));
    obj.position = avatar.position + direction * (avatar.radius + obj.radius);
    obj.color = avatar.color;
    obj.modifyVelocity(direction * m_config.explodeVelocity);
    obj.recombination(m_config.avatar.recombinationTime);
    avatar.player->addAvatar(&obj);
    if (avatars.size() >= m_config.player.maxCells) {
      break;
    }
  }
  if (explodedMass) {
    avatar.recombination(m_config.avatar.recombinationTime);
    avatar.modifyMass(-explodedMass);
  }
}

void Room::explode(Mother& mother)
{
  float doubleMass = m_config.mother.mass * 2;
  while (mother.mass >= doubleMass) {
    auto& obj = createMother();
    obj.modifyMass(m_config.mother.mass);
    float angle = (m_generator() % 3600) * M_PI / 1800;
    Vec2D direction(sin(angle), cos(angle));
    obj.position = mother.position;
    obj.modifyVelocity(direction * m_config.explodeVelocity);
    mother.modifyMass(-static_cast<float>(m_config.mother.mass));
  }
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

void Room::recalculateFreeSpace()
{
  m_hasFreeSpace = m_players.size() < m_config.maxPlayers;
}

void Room::updateNewCellRegistries(Cell* cell, bool checkRandomPos)
{
  m_createdCells.push_back(cell);
  m_activatedCells.insert(cell);
  m_modifiedCells.insert(cell);
  if (checkRandomPos) {
    m_forCheckRandomPos.insert(cell);
  }
}

void Room::removeCell(Cell* cell)
{
  spdlog::debug("Room::removeCell {}", cell->id);
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

void Room::resolveCellPosition(Cell& cell)
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

  expirationTime = currentTime - m_config.virus.lifeTime;
  std::erase_if(m_virusContainer, expired);

  expirationTime = currentTime - m_config.phage.lifeTime;
  std::erase_if(m_phageContainer, expired);

  expirationTime = currentTime - m_config.mother.lifeTime;
  std::erase_if(m_motherContainer, expired);
}

void Room::checkMothers()
{
  for (auto* mother : m_motherContainer) {
    auto radius = mother->radius + m_config.mother.checkRadius;
    Vec2D size(radius, radius);
    AABB boundingBox(mother->position - size, mother->position + size);
    mother->foodCount = m_gridmap.count(boundingBox);
  }
}

void Room::handlePlayerRequests()
{
  for (const auto& [sess, offset] : m_moveRequests) {
    if (auto* player = sess->player()) {
      player->setPointerOffset(offset);
    }
  }
  m_moveRequests.clear();

  for (const auto& [sess, point] : m_ejectRequests) {
    if (auto* player = sess->player()) {
      player->eject(point);
    }
  }
  m_ejectRequests.clear();

  for (const auto& [sess, point] : m_splitRequests) {
    if (auto* player = sess->player()) {
      player->split(point);
    }
  }
  m_splitRequests.clear();
}

void Room::update()
{
  auto now{TimePoint::clock::now()};
  auto deltaTime{now - m_lastUpdate};
  m_lastUpdate = now;
  ++m_tick;

  handlePlayerRequests();

  double dt = std::chrono::duration_cast<std::chrono::duration<double>>(deltaTime).count();

  for (auto* player : m_fighters) {
    player->applyPointerForce(m_tick);
    player->recombine(m_tick);
  }

  for (auto* cell : m_createdCells) {
    m_gridmap.insert(cell);
  }

  if (!m_activatedCells.empty()) {
    m_processingCells.insert(m_activatedCells.begin(), m_activatedCells.end());
    m_activatedCells.clear();
  }

  for (auto* cell : m_processingCells) {
    cell->applyResistanceForce();
    if (cell->force) {
      m_modifiedCells.insert(cell);
    }
    cell->simulate(dt);
    resolveCellPosition(*cell);
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

  synchronize(); // TODO: move to different timer
}

void Room::synchronize()
{
  std::erase_if(m_processingCells, // TODO: revise
    [this](Cell* cell)
    {
      if (cell->shouldBeProcessed()) {
        return false;
      }
      m_modifiedCells.insert(cell);
      return true;
    }
  );

  for (auto* player : m_fighters) {
    player->calcParams();
    player->synchronize(m_tick, m_modifiedCells, m_removedCellIds);
  }
  m_modifiedCells.clear();
  m_removedCellIds.clear();

  std::erase_if(m_fighters, [&](Player* player) { return player->isDead(); });

  if (!m_createdCells.empty()) {
    for (auto* cell : m_createdCells) {
      cell->newly = false;
    }
    m_createdCells.clear();
  }
}

void Room::updateLeaderboard()
{
  if (m_updateLeaderboard) {
    std::sort(m_leaderboard.begin(), m_leaderboard.end(), [] (Player* a, Player* b) { return *b < *a; });
    const auto& buffer = std::make_shared<Buffer>();
    OutgoingPacket::serializeLeaderboard(*buffer, m_leaderboard, m_config.leaderboard.limit);
    send(buffer);
    m_updateLeaderboard = false;
  }
}

void Room::produceMothers()
{
  if (m_mass >= m_config.maxMass || m_foodContainer.size() >= m_config.food.maxQuantity) {
    return;
  }

  auto velocityRange = m_config.food.maxVelocity - m_config.food.minVelocity;
  auto velocityDistribution = std::uniform_real_distribution<float>(
    m_config.food.minVelocity, m_config.food.minVelocity + velocityRange
  );
  auto angleDistribution = std::uniform_real_distribution<float>(0, 2 * M_PI);
  auto colorDistribution = std::uniform_int_distribution<int>(m_config.food.minColorIndex, m_config.food.maxColorIndex);

  for (auto* mother : m_motherContainer) {
    if (mother->foodCount >= m_config.mother.nearbyFoodLimit) {
      continue;
    }

    auto extraMassFactor = std::max(1.0f, (mother->mass - m_config.mother.mass) / m_config.mother.mass);
    auto foodToProduce = static_cast<int>(extraMassFactor * m_config.mother.baseFoodProduction);

    mother->foodCount += foodToProduce;

    for (int i = 0; i < foodToProduce; ++i) {
      auto angle = angleDistribution(m_generator);
      auto direction = Vec2D(sin(angle), cos(angle));
      auto& obj = createFood();
      obj.creator = mother;
      obj.position = mother->position + direction * mother->radius;
      obj.color = colorDistribution(m_generator);
      obj.modifyMass(m_config.food.mass);
      obj.modifyVelocity(direction * velocityDistribution(m_generator));
    }
  }
}

void Room::checkPlayers() // TODO: move to class Bot
{
  for (auto* bot : m_bots) {
    if (bot->isDead()) {
      auto& obj = createAvatar();
      obj.modifyMass(m_config.bot.mass);
      int radius = obj.radius;
      obj.position.x = (m_generator() % (m_config.width - 2 * radius)) + radius;
      obj.position.y = (m_generator() % (m_config.height - 2 * radius)) + radius;
      obj.color = m_generator() % 12;
      bot->addAvatar(&obj);
      bot->calcParams();
      bot->wakeUp();
      m_leaderboard.emplace_back(bot);
      m_fighters.insert(bot);
      sendPacketPlayerBorn(bot->getId());
    }
  }
}

void Room::generateFood()
{
  if (m_mass >= m_config.maxMass || m_foodContainer.size() > m_config.food.maxQuantity) {
    return;
  }
  auto availableSpace = static_cast<uint32_t>(m_config.food.maxQuantity - m_foodContainer.size());
  spawnFood(std::min(m_config.generator.food.quantity, availableSpace));
}

void Room::generateViruses()
{
  if (m_mass >= m_config.maxMass || m_virusContainer.size() > m_config.virus.maxQuantity) {
    return;
  }
  auto availableSpace = static_cast<uint32_t>(m_config.virus.maxQuantity - m_virusContainer.size());
  spawnViruses(std::min(m_config.generator.virus.quantity, availableSpace));
}

void Room::generatePhages()
{
  if (m_mass >= m_config.maxMass || m_phageContainer.size() > m_config.phage.maxQuantity) {
    return;
  }
  auto availableSpace = static_cast<uint32_t>(m_config.phage.maxQuantity - m_phageContainer.size());
  spawnPhages(std::min(m_config.generator.phage.quantity, availableSpace));
}

void Room::generateMothers()
{
  if (m_mass >= m_config.maxMass || m_motherContainer.size() > m_config.mother.maxQuantity) {
    return;
  }
  auto availableSpace = static_cast<uint32_t>(m_config.mother.maxQuantity - m_motherContainer.size());
  spawnMothers(std::min(m_config.generator.mother.quantity, availableSpace));
}

Player* Room::createPlayer(uint32_t id, const std::string& name)
{
  auto* player = new Player(m_executor, id, *this, m_gridmap);
  player->setName(name);
  player->subscribeToAnnihilationEvent(this,
    [this, player] {
     m_players.erase(player->getId());
     delete player;
    }
  ); // TODO: revise
  m_players.emplace(id, player);
  recalculateFreeSpace();
  return player;
}

void Room::createBots()
{
  uint32_t id = 100;
  for (const auto& name : m_config.botNames) {
    auto* bot = new Bot(m_executor, id++, *this, m_gridmap);
    bot->setName(name);
    bot->start();
    m_players.emplace(bot->getId(), bot);
    m_bots.emplace(bot);
  }
  recalculateFreeSpace();
}

void Room::spawnFood(uint32_t count)
{
  for (; count > 0; --count) {
    auto& obj = createFood();
    obj.position = getRandomPosition(obj.radius);
    obj.color = m_generator() % 16;
    obj.modifyMass(m_config.food.mass);
  }
}

void Room::spawnViruses(uint32_t count)
{
  for (; count > 0; --count) {
    auto& obj = createVirus();
    obj.modifyMass(m_config.virus.mass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::spawnPhages(uint32_t count)
{
  for (; count > 0; --count) {
    auto& obj = createPhage();
    obj.modifyMass(m_config.phage.mass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::spawnMothers(uint32_t count)
{
  for (; count > 0; --count) {
    auto& obj = createMother();
    obj.modifyMass(m_config.mother.mass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::serialize(Buffer& buffer)
{
  ::serialize(buffer, OutgoingPacket::Type::Room);
  ::serialize(buffer, static_cast<uint16_t>(m_config.width));
  ::serialize(buffer, static_cast<uint16_t>(m_config.height));
  ::serialize(buffer, static_cast<uint16_t>(m_config.viewportBase));
  ::serialize(buffer, m_config.viewportBuffer);
  ::serialize(buffer, m_config.aspectRatio);
  ::serialize(buffer, m_config.resistanceRatio);
  ::serialize(buffer, m_config.elasticityRatio);
  ::serialize(buffer, m_config.food.resistanceRatio);
  auto count = m_players.size();
  if (count > 255) {
    spdlog::warn("The number of players exceeds the limit of 255");
  }
  ::serialize(buffer, static_cast<uint8_t>(count));
  for (const auto& it : m_players) {
    Player* player = it.second;
    ::serialize(buffer, player->getId());
    ::serialize(buffer, player->getName());
    ::serialize(buffer, player->getStatus());
  }
  count = m_chatHistory.size();
  if (count > 255) {
    spdlog::warn("The size of chat history exceeds the limit of 255");
  }
  ::serialize(buffer, static_cast<uint8_t>(count));
  for (const auto& msg  : m_chatHistory) {
    ::serialize(buffer, msg.authorId);
    ::serialize(buffer, msg.author);
    ::serialize(buffer, msg.text);
  }
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
  OutgoingPacket::serializePlayer(*buffer, player);
  send(buffer);
}

void Room::sendPacketPlayerRemove(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializePlayerRemove(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerJoin(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializePlayerJoin(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerLeave(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializePlayerLeave(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerBorn(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializePlayerBorn(*buffer, playerId);
  send(buffer);
}

void Room::sendPacketPlayerDead(uint32_t playerId)
{
  const auto& buffer = std::make_shared<Buffer>();
  OutgoingPacket::serializePlayerDead(*buffer, playerId);
  send(buffer);
}

void Room::onAvatarDeath(Avatar* avatar)
{
  auto& player = *avatar->player;
  player.removeAvatar(avatar, nullptr); // TODO: revise

  m_updateLeaderboard = true;
  m_avatarContainer.erase(avatar);
  removeCell(avatar);

  if (player.isDead()) {
    sendPacketPlayerDead(player.getId());

    auto it = std::find(m_leaderboard.begin(), m_leaderboard.end(), &player);
    if (it != m_leaderboard.end()) {
      m_leaderboard.erase(it);
    }

    auto* observable = player.getKiller();
    if (!observable || observable->isDead()) {
      observable = m_leaderboard.empty() ? nullptr : *m_leaderboard.begin();
    }
    const auto& buffer = std::make_shared<Buffer>();
    if (observable) {
      OutgoingPacket::serializeSpectate(*buffer, *observable);
      for (const auto& sess : player.getSessions()) {
        observable->addSession(sess);
        sess->observable(observable);
      }
    } else {
      OutgoingPacket::serializeFinish(*buffer);
    }

    for (const auto& sess : player.getSessions()) {
      sess->send(buffer);
    }

    player.clearSessions(); // TODO: revise the functionality
  }
}

void Room::onFoodDeath(Food* food)
{
  m_foodContainer.erase(food);
  removeCell(food);
}

void Room::onBulletDeath(Bullet* bullet)
{
  m_bulletContainer.erase(bullet);
  removeCell(bullet);
}

void Room::onVirusDeath(Virus* virus)
{
  m_virusContainer.erase(virus);
  removeCell(virus);
}

void Room::onPhageDeath(Phage* phage)
{
  m_phageContainer.erase(phage);
  removeCell(phage);
}

void Room::onMotherDeath(Mother* mother)
{
  m_motherContainer.erase(mother);
  removeCell(mother);
}

void Room::onMotionStarted(Cell* cell)
{
  m_processingCells.insert(cell);
  m_modifiedCells.insert(cell);
}

void Room::onCellMassChange(Cell* cell, float deltaMass)
{
  m_mass += deltaMass;
  m_modifiedCells.insert(cell);
}

void Room::onAvatarMassChange(Avatar* avatar, float deltaMass)
{
  onCellMassChange(avatar, deltaMass);
  m_updateLeaderboard = true;
}

void Room::onMotherMassChange(Mother* mother, float deltaMass)
{
  onCellMassChange(mother, deltaMass);

  if (mother->mass >= m_config.mother.maxMass) {
    explode(*mother);
  }
}
