// file   : Room.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <fstream>

#include "Room.hpp"

#include "WebsocketServer.hpp"
#include "MemoryStream.hpp"
#include "Player.hpp"
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

#include "libs/chrono_io" // TODO: delete

Room::Room(uint32_t id, WebsocketServer& wss) :
  m_id(id),
  m_websocketServer(wss)
{
}

Room::~Room()
{
  for (auto&& it : m_players) {
    delete it.second;
  }
}

uint32_t Room::getId() const
{
  return m_id;
}

void Room::init(const RoomConfig& config)
{
  m_config = config;
  m_tickInterval = 0.001 * m_config.tickInterval.count();
  m_simulationInterval = m_tickInterval / m_config.simulationIterations;
  m_gridmap.resize(m_config.width, m_config.height, 9);
  spawnFood(m_config.foodStartAmount);
  spawnViruses(m_config.virusStartAmount);
  spawnPhages(m_config.phageStartAmount);
  spawnMothers(m_config.motherStartAmount);
  for (uint32_t i=0; i<m_config.botAmount; ++i) {
    spawnBot(101 + i);
  }
}

bool Room::hasFreeSpace() const
{
  // TODO: допиляти
  return m_players.size() < m_config.maxPlayers;
}

const RoomConfig& Room::getConfig() const
{
  return m_config;
}

void Room::join(const ConnectionHdl& hdl)
{
  if (!m_connections.emplace(hdl).second) {
    return;
  }

  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  const auto& user = conn->user;
  if (!user) {
    throw std::runtime_error("Room::join(): the user doesn't set");
  }

  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt16(m_config.width);
  ms.writeUInt16(m_config.height);
  ms.writeUInt16(m_config.viewportBase);
  ms.writeFloat(m_config.viewportBuffer);
  ms.writeFloat(m_config.aspectRatio);
  ms.writeFloat(m_config.resistanceRatio);
  ms.writeFloat(m_config.elasticityRatio);
  ms.writeFloat(m_config.foodResistanceRatio);
  auto count = m_players.size();
  if (count > 255) {
    LOG_WARN << "m_players.size() > 255";
  }
  ms.writeUInt8(count);
  for (auto&& it : m_players) {
    Player* player = it.second;
    ms.writeUInt32(player->getId());
    ms.writeString(player->name);
    ms.writeUInt8(player->getStatus());
  }
  count = m_chatHistory.size();
  if (count > 255) {
    LOG_WARN << "m_chat.size() > 255";
  }
  ms.writeUInt8(count);
  for (const auto& msg  : m_chatHistory) {
    ms.writeUInt32(msg.authorId);
    ms.writeString(msg.author);
    ms.writeString(msg.text);
  }
  packet.writeHeader(ms, OutputPacketTypes::Room);

  uint32_t playerId = 0;
  const auto& it = m_players.find(user->getId());
  if (it != m_players.end()) {
    Player* player = it->second;
    player->online = true;
    if (player->isDead()) {
      EmptyPacket packet(OutputPacketTypes::Finish);
      packet.format(ms);
    } else {
      player->addConnection(hdl);
      conn->player = player;
      PacketPlay packetPlay(*player);
      packetPlay.format(ms);
    }
    playerId = player->getId();
  } else {
    EmptyPacket packet(OutputPacketTypes::Finish);
    packet.format(ms);
  }
  m_websocketServer.send(hdl, ms);
  if (playerId) {
    sendPacketPlayerJoin(playerId);
    m_updateLeaderboard = true;
  }
}

void Room::leave(const ConnectionHdl& hdl)
{
  if (m_connections.erase(hdl)) {
    m_pointerRequests.erase(hdl);
    m_ejectRequests.erase(hdl);
    m_splitRequests.erase(hdl);
    // TODO: замінити код. (непотокобезпечний доступ до властивойстей conn)
    const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
    Player* player = conn->player;
    if (player) {
      sendPacketPlayerLeave(player->getId());
      conn->player = nullptr;
      m_zombiePlayers.insert(player);
      player->online = false;
    }
    // TODO: так як m_players не мають містить жодних з'єднань, то можна перебирати лише колекцію m_fighters
    for (auto&& it : m_players) {
      Player* player = it.second;
      player->removeConnection(hdl);
    }
  }
}

void Room::play(const ConnectionHdl& hdl, const std::string& name, uint8_t color)
{
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  const auto& user = conn->user;
  if (!user) {
    throw std::runtime_error("Room::play(): the user doesn't set");
  }

  uint32_t playerId = user->getId();
  Player* player = conn->player;
  if (!player) {
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = new Player(playerId, m_websocketServer, *this, m_gridmap);
      player->name = name;
      m_players.emplace(playerId, player);
      sendPacketPlayer(*player);
    }
    player->online = true;
    conn->player = player;
    sendPacketPlayerJoin(player->getId());
  }

  if (!player->isDead()) {
    return;
  }

  if (conn->observable) {
    conn->observable->removeConnection(hdl);
    conn->observable = nullptr;
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
  MemoryStream ms;
  PacketPlay packetPlay(*player);
  packetPlay.format(ms);
  m_websocketServer.send(hdl, ms);
  player->addConnection(hdl); // TODO: розібратись з додаваннями і забираннями hdl
}

void Room::spectate(const ConnectionHdl& hdl, uint32_t targetId)
{
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  const auto& user = conn->user;
  if (!user) {
    throw std::runtime_error("Room::spectate(): the user doesn't set");
  }

  uint32_t playerId = user->getId();
  Player* player = conn->player;
  if (!player) {
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = new Player(playerId, m_websocketServer, *this, m_gridmap);
      player->name = "Player " + std::to_string(user->getId());
      m_players.emplace(playerId, player);
      sendPacketPlayer(*player);
    }
    player->online = true;
    conn->player = player;
    sendPacketPlayerJoin(player->getId());
  }

  if (!player->isDead()) {
    return;
  }

  Player* target = nullptr;
  const auto& it = m_players.find(targetId);
  if (it != m_players.end()) {
    target = it->second;
  } else if (m_leaderboard.size()) {
    target = m_leaderboard.at(0);
  }
  if (m_fighters.find(target) == m_fighters.end()) {
    return;
  }
  if (player == target || conn->observable == target) {
    return;
  }
  if (player) {
    player->removeConnection(hdl);
  }
  if (conn->observable) {
    conn->observable->removeConnection(hdl);
  }
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  PacketSpectate packet(*target);
  packet.format(ms);
  m_websocketServer.send(hdl, ms);
  target->addConnection(hdl);
  conn->observable = target;
}

void Room::pointer(const ConnectionHdl& hdl, const Vec2D& point)
{
  m_pointerRequests.emplace(hdl, point);
}

void Room::eject(const ConnectionHdl& hdl, const Vec2D& point)
{
  m_ejectRequests.emplace(hdl, point);
}

void Room::split(const ConnectionHdl& hdl, const Vec2D& point)
{
  m_splitRequests.emplace(hdl, point);
}

void Room::chatMessage(const ConnectionHdl& hdl, const std::string& text)
{
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  Player* player = conn->player;
  if (!player){
    return;
  }
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(player->getId());
  ms.writeString(text);
  packet.writeHeader(ms, OutputPacketTypes::ChatMessage);
  if (text[0] == '#') {
    std::stringstream ss;
    packet.prepareHeader(ms);
    ms.writeUInt32(0);
    if (text == "#id") {
      ss << "id=" << player->getId();
    } else if (text == "#info") {
      float mass = 0;
      for (const auto& it : m_foodContainer) {
        mass += it.second.mass;
      }
      for (const auto& it : m_massContainer) {
        mass += it.second.mass;
      }
      for (const auto& it : m_virusContainer) {
        mass += it.second.mass;
      }
      for (const auto& it : m_phageContainer) {
        mass += it.second.mass;
      }
      for (const auto& it : m_motherContainer) {
        mass += it.second.mass;
      }
      for (const auto& it : m_avatarContainer) {
        mass += it.second.mass;
      }
      ss << "roomId=" << m_id << "; connections=" << m_connections.size()
        << "; players=" << m_players.size()
        << "; totalMass=" << m_mass << ":" << mass
        << "; avatars=" << m_avatarContainer.size()
        << "; food=" << m_foodContainer.size()
        << "; masses=" << m_massContainer.size()
        << "; viruses=" << m_virusContainer.size()
        << "; phages=" << m_phageContainer.size()
        << "; mothers=" << m_motherContainer.size();
    }
    ms.writeString(ss.str());
    packet.writeHeader(ms, OutputPacketTypes::ChatMessage);
  }
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
  m_chatHistory.emplace_front(player->getId(), player->name, text);
  while (m_chatHistory.size() > 128) {
    m_chatHistory.pop_back();
  }
}

void Room::watch(const ConnectionHdl& hdl, uint32_t playerId)
{
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  Player* player = conn->player;
  if (!player) {
    return;
  }
  const auto& it = m_players.find(playerId);
  if (it != m_players.end()) {
    Player* target = it->second;
    if (target != player) {
      player->arrowPlayer = target;
    }
  }
}

void Room::paint(const ConnectionHdl& hdl, const Vec2D& point)
{
  // TODO: add code here
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
  auto d = avatar.radius + food.radius;
  if (geometry::squareDistance(avatar.position, food.position) < d * d) {
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
  auto d = avatar.radius - 0.6 * virus.radius;
  if (geometry::squareDistance(avatar.position, virus.position) < d * d) {
    explode(avatar);
    m_zombieViruses.push_back(&virus);
    virus.zombie = true;
  }
}

void Room::interact(Avatar& avatar, Phage& phage)
{
  if (avatar.mass <= m_config.cellMinMass || avatar.mass < 1.25 * phage.mass) {
    return;
  }
  auto d = avatar.radius + phage.radius;
  if (geometry::squareDistance(avatar.position, phage.position) < d * d) {
    float m = std::min(m_simulationInterval * phage.mass, avatar.mass - m_config.cellMinMass);
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
  auto d = mother.radius - 0.25 * mass.radius;
  if (geometry::squareDistance(mother.position, mass.position) < d * d) {
    modifyMass(mother, mass.mass);
    m_activatedCells.insert(&mother);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Mother& mother, Virus& virus)
{
  auto d = mother.radius + virus.radius;
  if (geometry::squareDistance(mother.position, virus.position) < d * d) {
    modifyMass(mother, virus.mass);
    m_activatedCells.insert(&mother);
    m_zombieViruses.push_back(&virus);
    virus.zombie = true;
  }
}

void Room::interact(Mother& mother, Phage& phage)
{
  auto d = mother.radius + phage.radius;
  if (geometry::squareDistance(mother.position, phage.position) < d * d) {
    modifyMass(mother, phage.mass);
    m_activatedCells.insert(&mother);
    m_zombiePhages.push_back(&phage);
    phage.zombie = true;
  }
}

void Room::interact(Virus& virus, Food& food)
{
  auto d = virus.radius + food.radius;
  if (geometry::squareDistance(virus.position, food.position) < d * d) {
    m_zombieFoods.push_back(&food);
    food.zombie = true;
  }
}

void Room::interact(Virus& virus, Mass& mass)
{
  // TODO: реалізувати формулу пружнього зіткнення
  auto d = virus.radius + mass.radius;
  if (geometry::squareDistance(virus.position, mass.position) < d * d) {
    Vec2D direction((virus.position - mass.position).direction());
    virus.applyImpulse(direction * (mass.velocity.length() * mass.mass * m_config.massImpulseRatio));
    m_modifiedCells.insert(&virus);
    m_activatedCells.insert(&virus);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Virus& virus1, Virus& virus2)
{
  auto d = virus1.radius + virus2.radius;
  if (geometry::squareDistance(virus1.position, virus2.position) < d * d) {
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
  auto d = virus.radius + phage.radius;
  if (geometry::squareDistance(virus.position, phage.position) < d * d) {
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
  auto d = phage.radius + food.radius;
  if (geometry::squareDistance(phage.position, food.position) < d * d) {
    m_zombieFoods.push_back(&food);
    food.zombie = true;
  }
}

void Room::interact(Phage& phage, Mass& mass)
{
  // TODO: реалізувати формулу пружнього зіткнення
  auto d = phage.radius + mass.radius;
  if (geometry::squareDistance(phage.position, mass.position) < d * d) {
    Vec2D direction((phage.position - mass.position).direction());
    phage.applyImpulse(direction * (mass.velocity.length() * mass.mass * m_config.massImpulseRatio));
    m_modifiedCells.insert(&phage);
    m_activatedCells.insert(&phage);
    m_zombieMasses.push_back(&mass);
    mass.zombie = true;
  }
}

void Room::interact(Phage& phage1, Phage& phage2)
{
  auto d = phage1.radius + phage2.radius;
  if (geometry::squareDistance(phage1.position, phage2.position) < d * d) {
    auto& obj = createPhage();
    modifyMass(obj, phage1.mass + phage2.mass);
    obj.position = (phage1.position + phage2.position) * 0.5;
    m_zombiePhages.push_back(&phage1);
    m_zombiePhages.push_back(&phage2);
    phage1.zombie = true;
    phage2.zombie = true;
  }
}

void Room::recombination(Avatar& initiator, Avatar& target)
{
  auto dist = geometry::distance(initiator.position, target.position);
  auto direction((initiator.position - target.position).direction());
  if (initiator.isRecombined() && target.isRecombined()) {
    auto force = direction * ((initiator.mass + target.mass) * m_config.elasticityRatio);
    initiator.force -= force;
    target.force += force;
    m_modifiedCells.insert(&initiator);
    m_modifiedCells.insert(&target);
  } else {
    auto radius = initiator.radius + target.radius;
    if (dist < radius) {
      auto force = direction * ((initiator.mass + target.mass) * (radius - dist) * m_config.elasticityRatio);
      initiator.force += force;
      target.force -= force;
      m_modifiedCells.insert(&initiator);
      m_modifiedCells.insert(&target);
    }
  }
}

void Room::magnetism(Avatar& initiator, Avatar& target)
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

void Room::magnetism(Avatar& initiator, Food& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((target.position - initiator.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceFoodRatio / dist);
}

void Room::magnetism(Avatar& initiator, Mass& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((target.position - initiator.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceHungerRatio / dist);
}

void Room::magnetism(Avatar& initiator, Virus& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::magnetism(Avatar& initiator, Phage& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::magnetism(Avatar& initiator, Mother& target)
{
  float dist(geometry::squareDistance(target.position, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - target.position).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * ((initiator.mass + target.mass) * m_config.botForceStarRatio / dist);
}

void Room::magnetism(Avatar& initiator, const Vec2D& point)
{
  float dist(geometry::squareDistance(point, initiator.position));
  if (dist < m_config.eps) {
    return;
  }
  Vec2D velocity((initiator.position - point).direction() * initiator.maxSpeed);
  initiator.force += (velocity - initiator.velocity) * (initiator.mass * m_config.botForceCornerRatio / dist);
}

void Room::integration(Avatar& initiator, const Vec2D& point)
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
  auto now(TimePoint::clock::now());
  auto deltaTime(now - m_lastUpdate);
  if (deltaTime < m_config.tickInterval) {
    return;
  }
  auto tickCount = (deltaTime / m_config.tickInterval);
  m_tick += tickCount;
  m_lastUpdate += m_config.tickInterval * tickCount;
  auto dt = m_tickInterval * tickCount;

  destroyOutdatedCells();
  generate(dt);
  handlePlayerRequests();

  for (auto i = m_config.simulationIterations * tickCount; i > 0; --i) {
    simulate(m_simulationInterval);
  }

  auto deflationTime = now - std::chrono::seconds(m_config.playerDeflationTime);
  auto annihilationTime = now - std::chrono::seconds(m_config.playerAnnihilationTime);
  for (auto& it : m_avatarContainer) {
    Avatar& avatar = it.second;
    const auto& lastActivity = avatar.player->getLastActivity();
    if (lastActivity < deflationTime) {
      float mass = avatar.mass * m_config.playerDeflationRatio * dt;
      if (avatar.mass - mass >= m_config.cellMinMass) {
        modifyMass(avatar, -mass);
        m_updateLeaderboard = true;
      }
    }
    if (lastActivity < annihilationTime) {
      avatar.player->removeAvatar(&avatar);
      m_zombieAvatars.push_back(&avatar);
      avatar.zombie = true;
      m_updateLeaderboard = true;
      if (m_mass < m_config.maxMass) {
        auto& obj = createVirus();
        modifyMass(obj, avatar.mass > m_config.virusStartMass ? avatar.mass : m_config.virusStartMass);
        obj.position = avatar.position;
        obj.color = avatar.color;
      }
    }
  }

  for (Avatar* avatar : m_zombieAvatars) {
    m_processingAvatars.erase(avatar);
    removeCell(*avatar);
    m_avatarContainer.erase(avatar->id);
  }
  m_zombieAvatars.clear();
  for (Food* food : m_zombieFoods) {
    removeCell(*food);
    m_foodContainer.erase(food->id);
  }
  m_zombieFoods.clear();
  for (Mass* mass : m_zombieMasses) {
    removeCell(*mass);
    m_massContainer.erase(mass->id);
  }
  m_zombieMasses.clear();
  for (Virus* virus : m_zombieViruses) {
    removeCell(*virus);
    m_virusContainer.erase(virus->id);
  }
  m_zombieViruses.clear();
  for (Phage* phage : m_zombiePhages) {
    removeCell(*phage);
    m_phageContainer.erase(phage->id);
  }
  m_zombiePhages.clear();
  for (Mother* mother : m_zombieMothers) {
    removeCell(*mother);
    m_motherContainer.erase(mother->id);
  }
  m_zombieMothers.clear();

  // вибух гамнозірки
  std::vector<Mother*> mothers;
  mothers.reserve(m_motherContainer.size());
  for (auto&& it: m_motherContainer) {
    Mother& mother = it.second;
    if (mother.mass >= m_config.motherExplodeMass) {
      mothers.emplace_back(&mother);
    }
  }
  float doubleMass = m_config.motherStartMass * 2;
  for (Mother* mother : mothers) {
    while (mother->mass >= doubleMass) {
      auto& obj = createMother();
      modifyMass(obj, m_config.motherStartMass);
      float angle = (m_generator() % 3600) * M_PI / 1800;
      Vec2D direction(sin(angle), cos(angle));
      obj.position = mother->position;
      obj.applyImpulse(direction * (obj.mass * m_config.explodeImpulse));
      modifyMass(*mother, -static_cast<float>(m_config.motherStartMass));
    }
  }

  checkPlayers();
  synchronize();
  updateLeaderboard();

  // TODO: remove
  auto dur(TimePoint::clock::now() - now);
  // LOG_DEBUG << (0.001 * std::chrono::duration_cast<std::chrono::microseconds>(dur).count());
}

Vec2D Room::getRandomPosition(uint32_t radius) const
{
  Circle c;
  c.radius = radius;
  for (uint32_t n = m_config.spawnPosTryCount; n > 0; --n) {
    bool intersect = false;
    c.position.x = (m_generator() % (m_config.width - 2 * radius)) + radius;
    c.position.y = (m_generator() % (m_config.height - 2 * radius)) + radius;
    for (auto&& cell : m_forCheckRandomPos) {
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

// TODO: при видаленні клітки не використовується m_cellNextId.push
Avatar& Room::createAvatar()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_avatarContainer.emplace(id, *this).first->second;
  obj.id = id;
  m_createdAvatars.push_back(&obj);
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_forCheckRandomPos.insert(&obj);
  return obj;
}

Food& Room::createFood()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_foodContainer.emplace(id, *this).first->second;
  obj.id = id;
  obj.materialPoint = true;
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_activatedCells.insert(&obj);
  return obj;
}

Mass& Room::createMass()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_massContainer.emplace(id, *this).first->second;
  obj.id = id;
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_activatedCells.insert(&obj);
  return obj;
}

Virus& Room::createVirus()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_virusContainer.emplace(id, *this).first->second;
  obj.id = id;
  obj.color = m_config.virusColor;
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_activatedCells.insert(&obj);
  m_forCheckRandomPos.insert(&obj);
  return obj;
}

Phage& Room::createPhage()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_phageContainer.emplace(id, *this).first->second;
  obj.id = id;
  obj.color = m_config.phageColor;
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_activatedCells.insert(&obj);
  m_forCheckRandomPos.insert(&obj);
  return obj;
}

Mother& Room::createMother()
{
  uint32_t id = m_cellNextId.pop();
  auto& obj = m_motherContainer.emplace(id, *this).first->second;
  obj.id = id;
  obj.color = m_config.motherColor;
  m_createdCells.push_back(&obj);
  m_modifiedCells.insert(&obj);
  m_activatedCells.insert(&obj);
  m_forCheckRandomPos.insert(&obj);
  return obj;
}

void Room::removeCell(Cell& cell)
{
  m_mass -= cell.mass;
  m_forCheckRandomPos.erase(&cell);
  m_processingCells.erase(&cell);
  m_modifiedCells.erase(&cell);
  m_activatedCells.erase(&cell);
  m_gridmap.erase(&cell);
  m_removedCellIds.push_back(cell.id);
}

void Room::spawnBot(uint32_t id)
{
  const auto& it = m_players.find(id);
  Player* bot;
  if (it != m_players.end()) {
    bot = it->second;
  } else {
    bot = new Player(id, m_websocketServer, *this, m_gridmap);
    bot->name = "Bot " + std::to_string(id);
    bot->online = true;
    m_players.emplace(bot->getId(), bot);
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
    obj.applyImpulse(direction * obj.mass * m_config.avatarEjectImpulse);
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
    obj.applyImpulse(direction * (obj.mass * m_config.avatarSplitImpulse));
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
    obj.applyImpulse(direction * (obj.mass * m_config.explodeImpulse));
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

void Room::modifyMass(Cell& cell, float value)
{
  cell.mass += value;
  m_mass += value;
  if (cell.mass < m_config.cellMinMass) {
    LOG_WARN << "Bad cell mass. type=" << cell.type << ", mass=" << cell.mass << ", value=" << value;
    cell.mass = m_config.cellMinMass;
  }
  cell.radius = m_config.cellRadiusRatio * sqrt(cell.mass / M_PI);
  m_modifiedCells.insert(&cell);
}

void Room::modifyMass(Avatar& avatar, float value)
{
  modifyMass(static_cast<Cell&>(avatar), value);
  auto minRadius = m_config.cellRadiusRatio * sqrt(m_config.cellMinMass / M_PI);
  auto maxRadius = m_config.cellRadiusRatio * sqrt(m_config.maxMass / M_PI);
  auto speedDiff = m_config.avatarMaxSpeed - m_config.avatarMinSpeed;
  avatar.maxSpeed = m_config.avatarMaxSpeed - speedDiff * (avatar.radius - minRadius) / (maxRadius - minRadius);
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
  if (!m_config.destroyOutdatedCells || (m_tick - m_lastDestroyOutdatedCells < m_config.destroyOutdatedCells)) {
    return;
  }
  m_lastDestroyOutdatedCells += m_config.destroyOutdatedCells;
  auto now(TimePoint::clock::now());
  auto timePoint(now - std::chrono::seconds(m_config.virusLifeTime));
  for (auto it = m_virusContainer.begin(); it != m_virusContainer.end();) {
    auto& virus = it->second;
    if (virus.created < timePoint) {
      removeCell(virus);
      it = m_virusContainer.erase(it);
    } else {
      ++it;
    }
  }
  timePoint = now - std::chrono::seconds(m_config.phageLifeTime);
  for (auto it = m_phageContainer.begin(); it != m_phageContainer.end();) {
    auto& phage = it->second;
    if (phage.created < timePoint) {
      removeCell(phage);
      it = m_phageContainer.erase(it);
    } else {
      ++it;
    }
  }
  timePoint = now - std::chrono::seconds(m_config.motherLifeTime);
  for (auto it = m_motherContainer.begin(); it != m_motherContainer.end();) {
    auto& mother = it->second;
    if (mother.created < timePoint) {
      removeCell(mother);
      it = m_motherContainer.erase(it);
    } else {
      ++it;
    }
  }
}

void Room::generate(float dt)
{
  if (m_mass >= m_config.maxMass) {
    return;
  }

  int32_t avail = m_config.foodMaxAmount - m_foodContainer.size();
  if (avail > 0 && m_mass + m_accumulatedFoodMass < m_config.maxMass) {
    m_accumulatedFoodMass += m_config.spawnFoodMass * dt;
    int32_t count = std::min(static_cast<int>(m_accumulatedFoodMass / m_config.foodMass), avail);
    if (count > 0) {
      m_accumulatedFoodMass -= m_config.foodMass * count;
      spawnFood(count);
    }
  }

  avail = m_config.virusMaxAmount - m_virusContainer.size();
  if (avail > 0 && m_mass + m_accumulatedVirusMass < m_config.maxMass) {
    m_accumulatedVirusMass += m_config.spawnVirusMass * dt;
    int32_t count = std::min(static_cast<int>(m_accumulatedVirusMass / m_config.virusStartMass), avail);
    if (count > 0) {
      m_accumulatedVirusMass -= m_config.virusStartMass * count;
      spawnViruses(count);
    }
  }

  avail = m_config.phageMaxAmount - m_phageContainer.size();
  if (avail > 0 && m_mass + m_accumulatedPhageMass < m_config.maxMass) {
    m_accumulatedPhageMass += m_config.spawnPhageMass * dt;
    int32_t count = std::min(static_cast<int>(m_accumulatedPhageMass / m_config.phageStartMass), avail);
    if (count > 0) {
      m_accumulatedPhageMass -= m_config.phageStartMass * count;
      spawnPhages(count);
    }
  }

  avail = m_config.motherMaxAmount - m_motherContainer.size();
  if (avail > 0 && m_mass + m_accumulatedMotherMass < m_config.maxMass) {
    m_accumulatedMotherMass += m_config.spawnMotherMass * dt;
    int32_t count = std::min(static_cast<int>(m_accumulatedMotherMass / m_config.motherStartMass), avail);
    if (count > 0) {
      m_accumulatedMotherMass -= m_config.motherStartMass * count;
      spawnMothers(count);
    }
  }

  if (m_tick - m_lastCheckMothers >= m_config.checkMothers) {
    m_lastCheckMothers += m_config.checkMothers;
    for (auto& it : m_motherContainer) {
      auto& mother = it.second;
      Vec2D radius(mother.radius + m_config.motherCheckRadius, mother.radius + m_config.motherCheckRadius);
      AABB box(mother.position - radius, mother.position + radius);
      mother.foodCount = m_gridmap.count(box);
    }
  }
  mothersProduce();
}

void Room::handlePlayerRequests()
{
  websocketpp::lib::error_code ec;
  for (auto&& it : m_pointerRequests) {
    const auto& conn = m_websocketServer.get_con_from_hdl(it.first, ec);
    if (ec) {
      LOG_WARN << ec.message();
      continue;
    }
    Player* player = conn->player;
    if (player && !player->isDead()) {
      player->setPointer(it.second);
      const auto& avatars = player->getAvatars();
      m_modifiedCells.insert(avatars.begin(), avatars.end());
    }
  }
  m_pointerRequests.clear();

  for (auto&& it : m_ejectRequests) {
    const auto& conn = m_websocketServer.get_con_from_hdl(it.first, ec);
    if (ec) {
      LOG_WARN << ec.message();
      continue;
    }
    Player* player = conn->player;
    if (player && !player->isDead()) {
      for (Avatar* avatar : player->getAvatars()) {
        eject(*avatar, it.second);
      }
    }
  }
  m_ejectRequests.clear();

  for (auto&& it : m_splitRequests) {
    const auto& conn = m_websocketServer.get_con_from_hdl(it.first, ec);
    if (ec) {
      LOG_WARN << ec.message();
      continue;
    }
    Player* player = conn->player;
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
  for (Player* player : m_fighters) {
    auto& avatars = player->getAvatars();
    if (avatars.size() < 2) {
      continue;
    }
    for (auto it = avatars.begin(); it != avatars.end(); ++it) {
      Avatar& first = **it;
      if (first.zombie || first.protection > m_tick) {
        continue;
      }
      for (auto jt = it + 1; jt != avatars.end(); ++jt) {
        Avatar& second = **jt;
        if (second.zombie || second.protection > m_tick) {
          continue;
        }
        recombination(first, second);
      }
    }
  }

  // TODO: замінити на один прохід по m_botAvatars
  for (Player* bot : m_bots) {
    const auto& avatars = bot->getAvatars();
    bool applyPointerForce = avatars.size() > 1;
    Cell* eatTarget = nullptr;
    for (Avatar* avatar : avatars) {
      if (!avatar->zombie) {
        m_gridmap.query(avatar->player->getViewBox(), [&avatar, &eatTarget](Cell& target) -> bool {
          if (avatar != &target && !target.zombie) {
            target.magnetism(*avatar);
            if (target.isAttractive(*avatar)) {
              if (!eatTarget || eatTarget->mass < target.mass) {
                eatTarget = &target;
              }
            }
          }
          return true;
        });
        magnetism(*avatar, Vec2D(0, 0));
        magnetism(*avatar, Vec2D(m_config.width, 0));
        magnetism(*avatar, Vec2D(0, m_config.height));
        magnetism(*avatar, Vec2D(m_config.width, m_config.height));
        if (applyPointerForce) {
          magnetism(*avatar, bot->getPosition());
        }
        if (avatar->force) {
          m_modifiedCells.insert(avatar);
        }
      }
    }
    if (eatTarget) {
      bot->setPointer(eatTarget->position - bot->getPosition());
    }
  }

  for (Avatar* avatar : m_processingAvatars) {
    if (avatar->protection <= m_tick) {
      Vec2D direction(avatar->player->getDestination() - avatar->position);
      float dist = direction.length();
      float k = dist < avatar->radius ? dist / avatar->radius : 1;
      Vec2D velocity = direction.direction() * (k * avatar->maxSpeed);
      avatar->force += (velocity - avatar->velocity) * (avatar->mass * m_config.playerForceRatio);
      m_modifiedCells.insert(avatar);
    }
    avatar->simulate(dt);
    solveCellLocation(*avatar);
    m_gridmap.update(avatar);
  }
  for (Cell* cell : m_processingCells) {
    cell->simulate(dt);
    solveCellLocation(*cell);
    m_gridmap.update(cell);
  }
  for (Avatar* avatar : m_processingAvatars) {
    if (!avatar->zombie) {
      m_gridmap.query(avatar->getAABB(), [avatar](Cell& target) -> bool {
        if (avatar != &target && !avatar->zombie && !target.zombie) {
          avatar->interact(target);
        }
        return !avatar->zombie;
      });
    }
  }
  for (Cell* cell : m_processingCells) {
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
  for (auto it = m_processingCells.begin(); it != m_processingCells.end();) {
    Cell* cell = *it;
    if (!cell->velocity) {
      m_modifiedCells.insert(cell);
      it = m_processingCells.erase(it);
    } else {
      ++it;
    }
  }
  if (m_activatedCells.size()) {
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
  for (Player* player: m_fighters) {
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
      MemoryStream ms; // TODO: оптимізувати використання MemoryStream
      if (observable) {
        PacketSpectate packet(*observable);
        packet.format(ms);
        for (auto&& hdl : player->getConnections()) {
          const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
          m_websocketServer.send(hdl, ms);
          observable->addConnection(hdl);
          conn->observable = observable;
        }
      } else {
        EmptyPacket packet(OutputPacketTypes::Finish);
        packet.format(ms);
        for (auto&& hdl : player->getConnections()) {
          m_websocketServer.send(hdl, ms);
        }
      }
      player->clearConnections();
    }
  }
  m_leaderboard.erase(last, m_leaderboard.end());
  m_modifiedCells.clear();
  m_removedCellIds.clear();
  if (m_createdAvatars.size()) {
    m_processingAvatars.insert(m_createdAvatars.begin(), m_createdAvatars.end());
    m_createdAvatars.clear();
  }
  if (m_createdCells.size()) {
    for (Cell* cell : m_createdCells) {
      m_gridmap.insert(cell);
      cell->newly = false;
    }
    m_createdCells.clear();
  }

  for (auto it = m_fighters.begin(); it != m_fighters.end();) {
    Player* player = *it;
    if (player->isDead()) {
      it = m_fighters.erase(it);
    } else {
      ++it;
    }
  }
}

void Room::updateLeaderboard()
{
  if (!m_config.updateLeaderboard || (m_tick - m_lastUpdateLeaderboard >= m_config.updateLeaderboard)) {
    return;
  }
  m_lastUpdateLeaderboard += m_config.updateLeaderboard;
  if (m_updateLeaderboard) {
    std::sort(m_leaderboard.begin(), m_leaderboard.end(), [] (Player* a, Player* b) { return *b < *a; });
    MemoryStream ms; // TODO: оптимізувати використання MemoryStream
    PacketLeaderboard packetLeaderboard;
    packetLeaderboard.format(ms, m_leaderboard, m_config.leaderboardVisibleItems);
    for (auto&& hdl : m_connections) {
      m_websocketServer.send(hdl, ms);
    }
    m_updateLeaderboard = false;
  }
}

void Room::mothersProduce()
{
  if (!m_config.mothersProduce || (m_tick - m_lastMothersProduce < m_config.mothersProduce)) {
    return;
  }
  m_lastMothersProduce += m_config.mothersProduce;
  auto mothersCount = m_motherContainer.size();
  if (mothersCount == 0 || m_mass >= m_config.maxMass || m_foodContainer.size() >= m_config.foodMaxAmount) {
    return;
  }
  for (auto& it : m_motherContainer) {
    auto& mother = it.second;
    if (mother.foodCount >= 100) {
      continue;
    }
    int cnt = 0;
    int bonus = mother.mass - m_config.motherStartMass;
    if (bonus > 0) {
      cnt = bonus > 100 ? 20 : bonus > 25 ? 5 : 1;
      modifyMass(mother, -cnt);
    } else {
      cnt = mother.foodCount < 20 ? 5 : 1;
    }
    mother.foodCount += cnt;
    uint32_t impulse = m_config.foodMaxImpulse - m_config.foodMinImpulse;
    for (int i=0; i<cnt; ++i) {
      auto angle = M_PI * (m_generator() % 3600) / 1800;
      Vec2D direction(sin(angle), cos(angle));
      auto& obj = createFood();
      obj.creator = &mother;
      obj.position = mother.position;
      if (mother.radius > mother.startRadius) {
        obj.position += direction * (mother.radius - mother.startRadius);
      }
      obj.color = m_generator() % 16;
      obj.mass = m_config.foodMass;
      obj.radius = m_config.foodRadius;
      float k = obj.mass * (m_config.foodMinImpulse + (m_generator() % impulse));
      obj.applyImpulse(direction * k);
      m_mass += obj.mass;
    }
  }
}

void Room::checkPlayers()
{
  if (!m_config.checkPlayers || (m_tick - m_lastCheckPlayers < m_config.checkPlayers)) {
    return;
  }
  m_lastCheckPlayers += m_config.checkPlayers;
  for (Player* bot : m_bots) {
    if (bot->isDead()) {
      spawnBot(bot->getId());
    }
  }
  auto time = TimePoint::clock::now() - std::chrono::seconds(m_config.playerAnnihilationTime);
  for (auto it = m_zombiePlayers.begin(); it != m_zombiePlayers.end();) {
    Player* player = *it;
    if (player->isDead() && player->getLastActivity() < time) {
      it = m_zombiePlayers.erase(it);
      m_players.erase(player->getId());
      for (auto& jt : m_players) {
        Player* p = jt.second;
        p->removePlayer(player);
      }
    } else {
      ++it;
    }
  }
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

void Room::sendPacketPlayer(const Player& player)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(player.getId());
  ms.writeString(player.name);
  packet.writeHeader(ms, OutputPacketTypes::Player);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

void Room::sendPacketPlayerRemove(uint32_t playerId)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(playerId);
  packet.writeHeader(ms, OutputPacketTypes::PlayerRemove);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

void Room::sendPacketPlayerJoin(uint32_t playerId)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(playerId);
  packet.writeHeader(ms, OutputPacketTypes::PlayerJoin);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

void Room::sendPacketPlayerLeave(uint32_t playerId)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(playerId);
  packet.writeHeader(ms, OutputPacketTypes::PlayerLeave);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

void Room::sendPacketPlayerBorn(uint32_t playerId)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(playerId);
  packet.writeHeader(ms, OutputPacketTypes::PlayerBorn);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

void Room::sendPacketPlayerDead(uint32_t playerId)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(playerId);
  packet.writeHeader(ms, OutputPacketTypes::PlayerDead);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }
}

