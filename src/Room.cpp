// file   : src/Room.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Room.hpp"

#include "OutgoingPacket.hpp"
#include "Session.hpp"
#include "Player.hpp"
#include "Bot.hpp"

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
  , m_updateTimer(m_executor, [this] { update(); })
  , m_syncTimer(m_executor, [this] { synchronize(); })
  , m_updateLeaderboardTimer(m_executor, [this] { updateLeaderboard(); })
  , m_checkExpirableCellsTimer(m_executor, [this] { killExpiredCells(); })
  , m_updateNearbyFoodForMothersTimer(m_executor, [this] { updateNearbyFoodForMothers(); })
  , m_generateFoodByMothersTimer(m_executor, [this] { generateFoodByMothers(); })
  , m_foodGeneratorTimer(m_executor, [this] { generateFood(); })
  , m_virusGeneratorTimer(m_executor, [this] { generateViruses(); })
  , m_phageGeneratorTimer(m_executor, [this] { generatePhages(); })
  , m_motherGeneratorTimer(m_executor, [this] { generateMothers(); })
  , m_id(id)
{
}

Room::~Room()
{
  for (auto* cell : m_cells) {
    delete cell;
  }
}

void Room::init(const config::Room& config)
{
  m_config = config;

  m_updateTimer.setInterval(m_config.updateInterval);
  m_syncTimer.setInterval(m_config.syncInterval);
  m_updateLeaderboardTimer.setInterval(m_config.leaderboard.updateInterval);
  m_checkExpirableCellsTimer.setInterval(m_config.checkExpirableCellsInterval);
  m_updateNearbyFoodForMothersTimer.setInterval(m_config.mother.foodCheckInterval);
  m_generateFoodByMothersTimer.setInterval(m_config.mother.foodGenerationInterval);
  m_foodGeneratorTimer.setInterval(m_config.generator.food.interval);
  m_virusGeneratorTimer.setInterval(m_config.generator.virus.interval);
  m_phageGeneratorTimer.setInterval(m_config.generator.phage.interval);
  m_motherGeneratorTimer.setInterval(m_config.generator.mother.interval);

  m_gridmap.resize(m_config.width, m_config.height, 9);

  generateFood(m_config.food.quantity);
  generateViruses(m_config.virus.quantity);
  generatePhages(m_config.phage.quantity);
  generateMothers(m_config.mother.quantity);

  createBots();
}

void Room::start()
{
  m_updateTimer.start();
  m_syncTimer.start();
  m_updateLeaderboardTimer.start();
  m_checkExpirableCellsTimer.start();
  m_updateNearbyFoodForMothersTimer.start();
  m_generateFoodByMothersTimer.start();
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
  m_syncTimer.stop();
  m_updateLeaderboardTimer.stop();
  m_checkExpirableCellsTimer.stop();
  m_updateNearbyFoodForMothersTimer.stop();
  m_generateFoodByMothersTimer.stop();
  m_foodGeneratorTimer.stop();
  m_virusGeneratorTimer.stop();
  m_phageGeneratorTimer.stop();
  m_motherGeneratorTimer.stop();
  for (const auto& bot : m_bots) {
    bot->stop();
  }
}

bool Room::hasFreeSpace() const
{
  return m_hasFreeSpace;
}

void Room::join(const SessionPtr& sess, uint32_t playerId)
{
  asio::post(m_executor, std::bind_front(&Room::doJoin, this, sess, playerId));
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

Avatar& Room::createAvatar()
{
  auto* avatar = new Avatar(m_executor, *this, m_config, m_cellNextId.pop());
  avatar->subscribeToDeath(this, std::bind_front(&Room::onAvatarDeath, this, avatar));
  avatar->subscribeToMassChange(this, std::bind(&Room::onAvatarMassChange, this, avatar, _1));
  avatar->subscribeToMotionStarted(this, std::bind_front(&Room::onMotionStarted, this, avatar));
  avatar->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, avatar));
  updateNewCellRegistries(avatar);
  return *avatar;
}

Food& Room::createFood()
{
  auto* food = new Food(m_executor, *this, m_config, m_cellNextId.pop());
  food->subscribeToDeath(this, std::bind_front(&Room::onFoodDeath, this, food));
  food->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, food));
  updateNewCellRegistries(food, RegistryModificationOptions::None);
  ++m_foodQuantity;
  return *food;
}

Bullet& Room::createBullet()
{
  auto* bullet = new Bullet(m_executor, *this, m_config, m_cellNextId.pop());
  bullet->subscribeToDeath(this, std::bind_front(&Room::onBulletDeath, this, bullet));
  bullet->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, bullet));
  updateNewCellRegistries(bullet, RegistryModificationOptions::Activated);
  return *bullet;
}

Virus& Room::createVirus()
{
  auto* virus = new Virus(m_executor, *this, m_config, m_cellNextId.pop());
  virus->subscribeToDeath(this, std::bind_front(&Room::onVirusDeath, this, virus));
  virus->subscribeToMassChange(this, std::bind(&Room::onCellMassChange, this, virus, _1));
  virus->subscribeToMotionStarted(this, std::bind_front(&Room::onMotionStarted, this, virus));
  virus->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, virus));
  updateNewCellRegistries(virus);
  m_viruses.insert(virus);
  ++m_virusesQuantity;
  return *virus;
}

Phage& Room::createPhage()
{
  auto* phage = new Phage(m_executor, *this, m_config, m_cellNextId.pop());
  phage->subscribeToDeath(this, std::bind_front(&Room::onPhageDeath, this, phage));
  phage->subscribeToMassChange(this, std::bind(&Room::onCellMassChange, this, phage, _1));
  phage->subscribeToMotionStarted(this, std::bind_front(&Room::onMotionStarted, this, phage));
  phage->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, phage));
  updateNewCellRegistries(phage);
  m_phages.insert(phage);
  ++m_phagesQuantity;
  return *phage;
}

Mother& Room::createMother()
{
  auto* mother = new Mother(m_executor, *this, m_config, m_cellNextId.pop());
  mother->subscribeToDeath(this, std::bind_front(&Room::onMotherDeath, this, mother));
  mother->subscribeToMassChange(this, std::bind(&Room::onMotherMassChange, this, mother, _1));
  mother->subscribeToMotionStopped(this, std::bind_front(&Room::onMotionStopped, this, mother));
  updateNewCellRegistries(mother);
  m_mothers.insert(mother);
  ++m_mothersQuantity;
  return *mother;
}

std::random_device& Room::randomGenerator()
{
  return m_generator;
}

Vec2D Room::getRandomPosition(double radius) const
{
  auto xDistribution = std::uniform_int_distribution<>(radius, m_config.width - radius);
  auto yDistribution = std::uniform_int_distribution<>(radius, m_config.height - radius);
  Circle circle(radius);

  for (uint32_t n = m_config.spawnPosTryCount; n > 0; --n) {
    bool isIntersecting = false;
    circle.position.x = xDistribution(m_generator);
    circle.position.y = yDistribution(m_generator);
    for (const auto& cell : m_forRandomPositionCheck) {
      if (geometry::intersects(*cell, circle)) {
        isIntersecting = true;
        break;
      }
    }
    if (!isIntersecting) {
      break;
    }
  }
  return circle.position;
}

Vec2D Room::getRandomDirection() const
{
  static auto angleDistribution = std::uniform_real_distribution<float>(0, 2 * M_PI);
  auto angle = angleDistribution(m_generator);
  return {std::sin(angle), std::cos(angle)};
}

Gridmap& Room::getGridmap()
{
  return m_gridmap;
}

PlayerPtr Room::getTopPlayer() const
{
  return m_topPlayer.lock();
}

asio::any_io_executor& Room::getGameExecutor()
{
  return m_gameExecutor;
}

asio::any_io_executor& Room::getDeathExecutor()
{
  return m_deathExecutor;
}

void Room::doJoin(const SessionPtr& sess, uint32_t playerId)
{
  if (!m_sessions.emplace(sess).second) {
    return;
  }

  sess->playerId(playerId);

  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer);

  const auto& it = m_players.find(playerId);
  if (it != m_players.end()) {
    const auto& player = it->second;
    player->setMainSession(sess);
    sendPacketPlayerJoin(playerId);
    OutgoingPacket::serializePlay(*buffer, *player);
  } else {
    OutgoingPacket::serializeFinish(*buffer);
  }

  sess->send(buffer);
}

void Room::doLeave(const SessionPtr& sess)
{
  if (m_sessions.erase(sess)) {
    m_moveRequests.erase(sess);
    m_ejectRequests.erase(sess);
    m_splitRequests.erase(sess);
    if (const auto& player = sess->player()) {
      player->removeSession(sess);
      sendPacketPlayerLeave(player->getId());
    }
    if (const auto& observable = sess->observable()) {
      observable->removeSession(sess);
      sess->observable(nullptr);
    }
  }
}

void Room::doPlay(const SessionPtr& sess, const std::string& name, uint8_t color)
{
  if (const auto& observable = sess->observable()) {
    observable->removeSession(sess);
    sess->observable(nullptr);
  }

  auto player = sess->player();

  if (!player) {
    uint32_t playerId = sess->playerId();
    const auto& it = m_players.find(playerId);
    if (it != m_players.end()) {
      player = it->second;
    } else {
      player = createPlayer(playerId, name);
    }
  }

  player->setMainSession(sess);

  if (player->isDead()) {
    player->setColor(color);
    if (player->getName() != name) {
      player->setName(name);
    }
    player->respawn();
  }

  sendPacketPlayer(*player);
  sendPacketPlayerJoin(player->getId());
  sendPacketPlayerBorn(player->getId());
}

void Room::doSpectate(const SessionPtr& sess, uint32_t targetId)
{
  const auto& player = sess->player();

  if (player && !player->isDead()) {
    return;
  }

  PlayerPtr target;
  const auto& it = m_players.find(targetId);
  if (it != m_players.end()) {
    target = it->second;
  } else if (!m_leaderboard.empty()) {
    target = m_leaderboard.at(0);
  }
  if (!m_fighters.contains(target)) {
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
  auto player = sess->player();
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
  auto player = sess->player();
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

void Room::recalculateFreeSpace()
{
  m_hasFreeSpace = m_players.size() < m_config.maxPlayers;
}

void Room::updateNewCellRegistries(Cell* cell, RegistryModificationOptions options)
{
  m_cells.insert(cell);
  m_newCells.insert(cell);
  m_createdCells.insert(cell);
  if (options & RegistryModificationOptions::Activated) {
    m_activatedCells.insert(cell);
  }
  m_modifiedCells.insert(cell);
  if (options & RegistryModificationOptions::ForRandomPositionCheck) {
    m_forRandomPositionCheck.insert(cell);
  }
}

void Room::prepareCellForDestruction(Cell* cell)
{
  m_deadCells.push_back(cell);
  m_processingCells.erase(cell);
  m_forRandomPositionCheck.erase(cell);
  m_newCells.erase(cell);
  m_createdCells.erase(cell);
  m_activatedCells.erase(cell);
  m_modifiedCells.erase(cell);
  m_gridmap.erase(cell);
}

void Room::removeCell(Cell* cell)
{
  m_mass -= cell->mass;
  m_cellNextId.push(cell->id);
  m_cells.erase(cell);
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

void Room::killExpiredCells()
{
  auto currentTime = TimePoint::clock::now();
  auto killExpired = [](auto& container, const TimePoint& expirationTime)
    {
      for (auto* cell : container) {
        if (cell->created < expirationTime) {
          cell->kill();
        };
      }
    };

  killExpired(m_viruses, currentTime - m_config.virus.lifeTime);
  killExpired(m_phages, currentTime - m_config.phage.lifeTime);
  killExpired(m_mothers, currentTime - m_config.mother.lifeTime);
}

void Room::handlePlayerRequests()
{
  for (const auto& [sess, offset] : m_moveRequests) {
    if (const auto& player = sess->player()) {
      player->setPointerOffset(offset);
    }
  }
  m_moveRequests.clear();

  for (const auto& [sess, point] : m_ejectRequests) {
    if (const auto& player = sess->player()) {
      player->eject(point);
    }
  }
  m_ejectRequests.clear();

  for (const auto& [sess, point] : m_splitRequests) {
    if (const auto& player = sess->player()) {
      player->split(point);
    }
  }
  m_splitRequests.clear();
}

void Room::update()
{
  auto now{TimePoint::clock::now()};
  double dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_lastUpdate).count();
  m_lastUpdate = now;

  handlePlayerRequests();

  for (const auto& player : m_fighters) {
    player->applyPointerForce();
    player->recombine();
  }

  if (!m_createdCells.empty()) {
    for (auto* cell: m_createdCells) {
      resolveCellPosition(*cell);
      m_gridmap.insert(cell);
      if (cell->velocity) {
        m_processingCells.insert(cell);
      }
    }
    m_createdCells.clear();
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

  m_gameContext.run();
  m_gameContext.restart();
  m_deathContext.run();
  m_deathContext.restart();
}

void Room::synchronize()
{
  std::vector<uint32_t> removedCellIds;
  removedCellIds.reserve(m_deadCells.size());
  for (auto* cell : m_deadCells) {
    removedCellIds.push_back(cell->id);
  }

  for (const auto& player : m_fighters) {
    player->calcParams();
    player->synchronize(m_modifiedCells, removedCellIds);
  }
  m_modifiedCells.clear();

  std::erase_if(m_fighters, [&](const auto& player) { return player->isDead(); });

  for (auto* cell : m_newCells) {
    cell->newly = false;
  }
  m_newCells.clear();

  for (auto* cell : m_deadCells) {
    removeCell(cell);
  }
  m_deadCells.clear();
}

void Room::updateLeaderboard()
{
  if (m_updateLeaderboard) {
    if (m_leaderboard.empty()) {
      m_topPlayer.reset();
    } else {
      std::sort(m_leaderboard.begin(), m_leaderboard.end(), [](const auto& a, const auto& b) { return *b < *a; });
      m_topPlayer = m_leaderboard[0];
    }
    const auto& buffer = std::make_shared<Buffer>();
    OutgoingPacket::serializeLeaderboard(*buffer, m_leaderboard, m_config.leaderboard.limit);
    send(buffer);
    m_updateLeaderboard = false;
  }
}

void Room::removeFromLeaderboard(const PlayerPtr& player)
{
  auto it = std::find(m_leaderboard.begin(), m_leaderboard.end(), player);
  if (it != m_leaderboard.end()) {
    m_leaderboard.erase(it);
    m_updateLeaderboard = true;
  }
}

void Room::updateNearbyFoodForMothers()
{
  for (auto* mother : m_mothers) {
    mother->calculateNearbyFood();
  }
}

void Room::generateFoodByMothers()
{
  for (auto* mother : m_mothers) {
    if (m_mass >= m_config.maxMass || m_foodQuantity >= m_config.food.maxQuantity) {
      return;
    }
    mother->generateFood();
  }
}

PlayerPtr Room::createPlayer(uint32_t id, const std::string& name)
{
  auto player = std::make_shared<Player>(m_executor, *this, m_config, id);
  player->setName(name);
  auto weakPlayer = std::weak_ptr(player);
  player->subscribeToRespawn(this, std::bind_front(&Room::onPlayerRespawn, this, weakPlayer));
  player->subscribeToDeath(this, std::bind_front(&Room::onPlayerDeath, this, weakPlayer));
  player->subscribeToAnnihilation(this, std::bind_front(&Room::onPlayerAnnihilates, this, weakPlayer));
  m_players.emplace(id, player);
  sendPacketPlayer(*player);
  recalculateFreeSpace();
  return player;
}

void Room::createBots()
{
  static auto colorIndexDistribution = std::uniform_int_distribution<uint8_t>(0, 12);

  uint32_t id = 100;
  for (const auto& name : m_config.botNames) {
    auto bot = std::make_shared<Bot>(m_executor, *this, m_config, id++);
    bot->subscribeToRespawn(this, std::bind_front(&Room::onPlayerRespawn, this, bot));
    bot->subscribeToDeath(this, std::bind_front(&Room::onPlayerDeath, this, bot));
    bot->setName(name);
    bot->setColor(colorIndexDistribution(m_generator));
    bot->respawn();
    bot->start();
    m_players.emplace(bot->getId(), bot);
    m_bots.emplace(bot);
    sendPacketPlayer(*bot);
  }
  recalculateFreeSpace();
}

void Room::generateFood()
{
  if (m_mass >= m_config.maxMass || m_foodQuantity > m_config.food.maxQuantity) {
    return;
  }
  auto quantity = m_config.food.maxQuantity - m_foodQuantity;
  generateFood(std::min(m_config.generator.food.quantity, quantity));
}

void Room::generateViruses()
{
  if (m_mass >= m_config.maxMass || m_virusesQuantity > m_config.virus.maxQuantity) {
    return;
  }
  auto quantity = m_config.virus.maxQuantity - m_virusesQuantity;
  generateViruses(std::min(m_config.generator.virus.quantity, quantity));
}

void Room::generatePhages()
{
  if (m_mass >= m_config.maxMass || m_phagesQuantity > m_config.phage.maxQuantity) {
    return;
  }
  auto quantity = m_config.phage.maxQuantity - m_phagesQuantity;
  generatePhages(std::min(m_config.generator.phage.quantity, quantity));
}

void Room::generateMothers()
{
  if (m_mass >= m_config.maxMass || m_mothersQuantity > m_config.mother.maxQuantity) {
    return;
  }
  auto quantity = static_cast<uint32_t>(m_config.mother.maxQuantity - m_mothersQuantity);
  generateMothers(std::min(m_config.generator.mother.quantity, quantity));
}

void Room::generateFood(int quantity)
{
  auto colorIndexDistribution = std::uniform_int_distribution<uint8_t>(
    m_config.food.minColorIndex, m_config.food.maxColorIndex
  );
  while (--quantity >= 0) {
    auto& obj = createFood();
    obj.position = getRandomPosition(obj.radius);
    obj.color = colorIndexDistribution(m_generator);
    obj.modifyMass(m_config.food.mass);
  }
}

void Room::generateViruses(int quantity)
{
  while (--quantity >= 0) {
    auto& obj = createVirus();
    obj.modifyMass(m_config.virus.mass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::generatePhages(int quantity)
{
  while (--quantity >= 0) {
    auto& obj = createPhage();
    obj.modifyMass(m_config.phage.mass);
    obj.position = getRandomPosition(obj.radius);
  }
}

void Room::generateMothers(int quantity)
{
  while (--quantity >= 0) {
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
    const auto& player = it.second;
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

void Room::onPlayerRespawn(const PlayerWPtr& weakPlayer)
{
  if (const auto& player = weakPlayer.lock()) {
    m_leaderboard.emplace_back(player);
    m_updateLeaderboard = true;
    m_fighters.insert(player);
    sendPacketPlayerBorn(player->getId());
  }
}

void Room::onPlayerDeath(const PlayerWPtr& weakPlayer)
{
  if (const auto& player = weakPlayer.lock()) {
    removeFromLeaderboard(player);
    m_fighters.erase(player);
    sendPacketPlayerDead(player->getId());
  }
}

void Room::onPlayerAnnihilates(const PlayerWPtr& weakPlayer)
{
  if (const auto& player = weakPlayer.lock()) {
    m_players.erase(player->getId());
  }
}

void Room::onAvatarDeath(Avatar* avatar)
{
  prepareCellForDestruction(avatar);
}

void Room::onFoodDeath(Food* food)
{
  --m_foodQuantity;
  prepareCellForDestruction(food);
}

void Room::onBulletDeath(Bullet* bullet)
{
  prepareCellForDestruction(bullet);
}

void Room::onVirusDeath(Virus* virus)
{
  --m_virusesQuantity;
  prepareCellForDestruction(virus);
  m_viruses.erase(virus);
}

void Room::onPhageDeath(Phage* phage)
{
  --m_phagesQuantity;
  prepareCellForDestruction(phage);
  m_phages.erase(phage);
}

void Room::onMotherDeath(Mother* mother)
{
  --m_mothersQuantity;
  prepareCellForDestruction(mother);
  m_mothers.erase(mother);
}

void Room::onMotionStarted(Cell* cell)
{
  m_processingCells.insert(cell);
  m_modifiedCells.insert(cell);
}

void Room::onMotionStopped(Cell* cell)
{
  m_processingCells.erase(cell);
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
    mother->explode();
  }
}
