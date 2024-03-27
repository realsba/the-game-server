// file   : src/Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Player.hpp"

#include "serialization.hpp"
#include "OutgoingPacket.hpp"
#include "Session.hpp"
#include "Room.hpp"

#include "entity/Cell.hpp"
#include "entity/Avatar.hpp"
#include "geometry/geometry.hpp"

#include <spdlog/spdlog.h>

Player::Player(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : m_deflationTimer(executor)
  , m_annihilationTimer(executor)
  , m_annihilationEmitter(executor)
  , m_deathEmitter(executor)
  , m_respawnEmitter(executor)
  , m_entityFactory(entityFactory)
  , m_config(config)
  , m_gridmap(entityFactory.getGridmap())
  , m_id(id)
{
  wakeUp();
}

uint32_t Player::getId() const
{
  return m_id;
}

std::string Player::getName() const
{
  return m_name;
}

uint32_t Player::getMass() const
{
  return m_mass;
}

uint32_t Player::getMaxMass() const
{
  return m_maxMass;
}

const AABB& Player::getViewBox() const
{
  return m_viewbox;
}

Vec2D Player::getPosition() const
{
  return m_position;
}

const Avatars& Player::getAvatars() const
{
  return m_avatars;
}

Avatar* Player::findTheBiggestAvatar() const
{
  if (m_avatars.empty()) {
    return nullptr;
  }

  auto maxElementIt = std::max_element(
    m_avatars.begin(), m_avatars.end(), [](const Cell* a, const Cell* b) { return a->mass < b->mass; }
  );

  return *maxElementIt;
}

PlayerPtr Player::getKiller() const
{
  return m_killer.lock();
}

bool Player::isDead() const
{
  return !m_status.isAlive;
}

uint8_t Player::getStatus() const
{
  uint8_t status = 0;
  status |= (m_status.isOnline ? 1 : 0) << 0;
  status |= (m_status.isAlive ? 1 : 0) << 1;
  return status;
}

const Sessions& Player::getSessions() const
{
  return m_sessions;
}

void Player::respawn()
{
  if (m_status.isAlive) {
    return;
  }

  auto& avatar = m_entityFactory.createAvatar();
  avatar.modifyMass(m_config.player.mass);
  avatar.position = m_entityFactory.getRandomPosition(avatar.radius);
  avatar.color = m_color;
  addAvatar(&avatar);
  calcParams(); // TODO: do not use
  wakeUp();

  if (m_mainSession) {
    const auto& buffer = std::make_shared<Buffer>();
    OutgoingPacket::serializePlay(*buffer, *this);
    m_mainSession->send(buffer);
  }

  m_respawnEmitter.emit();
}

void Player::setName(const std::string& name)
{
  m_name = name;
}

void Player::setColor(uint8_t color)
{
  m_color = color;
}

void Player::setPointerOffset(const Vec2D& value)
{
  if (m_status.isAlive && value) {
    m_pointerOffset = value;
    startMotion();
  } else {
    m_pointerOffset.zero();
  }
}

void Player::setMainSession(const SessionPtr& sess)
{
  if (sess) {
    if (m_mainSession) {
      m_sessions.erase(m_mainSession);
      m_mainSession->player(nullptr);
    }
    m_mainSession = sess;
    m_mainSession->player(shared_from_this());
    m_status.isOnline = true;
    addSession(sess);
  }
}

void Player::addSession(const SessionPtr& sess)
{
  if (m_sessions.emplace(sess).second) { // TODO: revise, make sense to send initial state only to new sess
    m_leftTopSector = nullptr;
    m_rightBottomSector = nullptr;
    m_sectors.clear();
    m_visibleIds.clear();
  }
}

void Player::removeSession(const SessionPtr& sess)
{
  if (m_sessions.erase(sess)) {
    if (m_mainSession == sess) {
      m_mainSession->player(nullptr);
      m_mainSession.reset();
      m_status.isOnline = false;
      m_pointerOffset.zero();
    }
  }
}

void Player::clearSessions()
{
  if (m_mainSession) {
    m_mainSession->player(nullptr);
    m_mainSession.reset();
  }
  m_sessions.clear();
}

void Player::setTargetPlayer(const PlayerPtr& player)
{
  if (player.get() != this) {
    m_targetPlayer = player;
  }
}

void Player::eject(const Vec2D& point)
{
  if (!m_status.isAlive) {
    return;
  }
  for (auto* avatar : m_avatars) {
    avatar->eject(point);
  }
}

void Player::split(const Vec2D& point)
{
  if (!m_status.isAlive) {
    return;
  }

  size_t count = m_avatars.size();
  for (auto* avatar : m_avatars) {
    if (count >= m_config.player.maxCells) {
      break;
    }
    if (avatar->split(point)) {
      ++count;
    }
  }
}

void Player::synchronize(const std::set<Cell*>& modified, const std::vector<uint32_t>& removed)
{
  AABB viewport(m_gridmap.clip(m_viewport));
  auto* leftTop = m_gridmap.getSector(viewport.a);
  auto* rightBottom = m_gridmap.getSector(viewport.b);
  if (!leftTop || !rightBottom) {
    throw std::runtime_error("Invalid sector");
  }

  bool sectorsChanged = false;
  if (m_leftTopSector != leftTop || m_rightBottomSector != rightBottom) {
    m_leftTopSector = leftTop;
    m_rightBottomSector = rightBottom;
    m_viewbox.a = leftTop->box.a;
    m_viewbox.b = rightBottom->box.b;
    sectorsChanged = true;
  }

  if (m_sessions.empty()) {
    return;
  }

  std::set<Cell*> syncCells;
  std::set<uint32_t> removedIds;

  if (sectorsChanged) {
    const auto& sectors = m_gridmap.getSectors(viewport);
    std::erase_if(m_sectors,
      [&](Sector* sector)
      {
        if (sectors.find(sector) == sectors.end()) {
          for (Cell* cell : sector->cells) {
            if (!cell->intersects(m_viewbox)) {
              removedIds.insert(cell->id);
            }
          }
          return true;
        }
        return false;
      }
    );
    for (Sector* sector : sectors) {
      const auto& res = m_sectors.insert(sector);
      if (res.second) {
        syncCells.insert(sector->cells.begin(), sector->cells.end());
      }
    }
  }
  for (Cell* cell : modified) {
    if (cell->player == this || cell->intersects(m_viewbox)) {
      syncCells.insert(cell);
    }
  }
  for (auto id : removed) {
    if (m_visibleIds.erase(id)) {
      removedIds.insert(id);
    }
  }

  const auto& buffer = std::make_shared<Buffer>();
  serialize(*buffer, OutgoingPacket::Type::Frame);
  serialize(*buffer, m_scale);
  serialize(*buffer, static_cast<uint16_t>(syncCells.size()));
  for (Cell* cell : syncCells) {
    cell->format(*buffer);
    m_visibleIds.insert(cell->id);
  }
  serialize(*buffer, static_cast<uint16_t>(removedIds.size()));
  for (auto id : removedIds) {
    serialize(*buffer, id);
  }
  serialize(*buffer, static_cast<uint8_t>(m_avatars.size()));
  for (const Avatar* avatar : m_avatars) {
    serialize(*buffer, avatar->id);
    serialize(*buffer, avatar->getMaxVelocity());
  }
  if (auto targetPlayer = m_targetPlayer.lock()) {
    serialize(*buffer, targetPlayer->getId());
    const auto& position = targetPlayer->getPosition();
    serialize(*buffer, position.x);
    serialize(*buffer, position.y);
  } else {
    serialize(*buffer, static_cast<uint32_t>(0));
  }
  for (const auto& session : m_sessions) {
    session->send(buffer);
  }

  syncCells.clear();
  removedIds.clear();
}

void Player::wakeUp()
{
  scheduleDeflation();
  scheduleAnnihilation();
}

void Player::calcParams()
{
  m_mass = 0;
  if (m_avatars.empty()) {
    return;
  }
  Vec2D position;
  float radius = 0;
  float mass = 0;
  for (const Avatar* avatar : m_avatars) {
    position += avatar->position * avatar->mass;
    mass += avatar->mass;
    if (avatar->radius > radius) {
      radius = avatar->radius;
    }
  }
  if (mass != 0) {
    position /= mass;
  }
  m_mass = static_cast<uint32_t>(mass);
  if (m_mass > m_maxMass) {
    m_maxMass = m_mass;
  }
  float scale = radius > m_config.maxRadius ? 1 + m_config.scaleRatio * (radius / m_config.maxRadius - 1) : 1;
  if (m_scale != scale) {
    m_scale = scale;
    auto halfHeight = 0.5 * m_config.viewportBase * (1 + 2 * m_config.viewportBuffer) * m_scale;
    auto halfWidth = halfHeight * m_config.aspectRatio;
    Vec2D buffer(halfWidth, halfHeight);
    m_viewport.a = position - buffer;
    m_viewport.b = position + buffer;
  } else {
    m_viewport.translate(position - m_position);
  }
  m_position = position;
}

void Player::applyPointerForce()
{
  if (!m_pointerOffset) {
    return;
  }
  auto destination = m_position + m_pointerOffset;
  for (auto* avatar : m_avatars) {
    avatar->applyPointAttractionForce(destination, m_config.player.pointerForceRatio);
  }
}

void Player::recombine()
{
  if (m_avatars.size() < 2) {
    return;
  }
  for (auto it = m_avatars.begin(); it != m_avatars.end(); ++it) {
    Avatar& first = **it;
    if (first.zombie) {
      continue;
    }
    for (auto jt = std::next(it); jt != m_avatars.end(); ++jt) {
      Avatar& second = **jt;
      if (second.zombie) {
        continue;
      }
      recombine(first, second);
    }
  }
}

void Player::subscribeToAnnihilation(void* tag, EventEmitter<>::Handler&& handler)
{
  m_annihilationEmitter.subscribe(tag, std::move(handler));
}

void Player::subscribeToRespawn(void* tag, EventEmitter<>::Handler&& handler)
{
  m_respawnEmitter.subscribe(tag, std::move(handler));
}

void Player::subscribeToDeath(void* tag, EventEmitter<>::Handler&& handler)
{
  m_deathEmitter.subscribe(tag, std::move(handler));
}

void Player::addAvatar(Avatar* avatar)
{
  spdlog::debug("Player::addAvatar {}", avatar->id);

  avatar->player = this;
  m_avatars.emplace(avatar);
  m_status.isAlive = true;

  //TODO: implement params calculating mass && position
  // m_position = (m_position * (m_mass - newAvatar->mass) + newAvatar->position * newAvatar->mass) / m_mass;

  avatar->subscribeToAvatarCreation(this, std::bind(&Player::addAvatar, this, std::placeholders::_1));
  avatar->subscribeToDeath(this, std::bind_front(&Player::removeAvatar, this, avatar));
}

void Player::removeAvatar(Avatar* avatar)
{
  spdlog::debug("Player::removeAvatar {}", avatar->id);
  m_avatars.erase(avatar);
  if (m_avatars.empty()) {
    m_status.isAlive = false;
// TODO: implement
//    if (killer) {
//      if (m_killer) {
//        m_killer->unsubscribeFromAnnihilation(this);
//      }
//      m_killer = killer;
//      m_killer->subscribeToAnnihilation(this, [this] { m_killer = nullptr; });
//    }
    m_deathEmitter.emit();
  }
}

void Player::recombine(Avatar& initiator, Avatar& target)
{
  auto elasticityRatio = m_config.elasticityRatio;
  auto direction((initiator.position - target.position).direction());
  if (initiator.isRecombined() && target.isRecombined()) {
    auto force = direction * ((initiator.mass + target.mass) * elasticityRatio);
    initiator.force -= force;
    target.force += force;
  } else {
    auto distance = geometry::distance(initiator.position, target.position);
    auto radius = initiator.radius + target.radius;
    if (distance < radius) {
      auto force = direction * ((initiator.mass + target.mass) * (radius - distance) * elasticityRatio);
      initiator.force += force;
      target.force -= force;
    }
  }
}

void Player::scheduleDeflation()
{
  m_deflationTimer.expires_after(m_config.player.deflationThreshold);
  m_deflationTimer.async_wait([this](const boost::system::error_code &error) {
    if (!error) {
      handleDeflation();
    }
  });
}

void Player::scheduleAnnihilation()
{
  m_annihilationTimer.expires_after(m_config.player.annihilationThreshold);
  m_annihilationTimer.async_wait([this](const boost::system::error_code& error) {
    if (!error) {
      handleAnnihilation();
    }
  });
}

void Player::handleDeflation()
{
  m_deflationTimer.expires_after(m_config.player.deflationInterval);
  m_deflationTimer.async_wait([this](const boost::system::error_code& error) {
    if (!error) {
      for (auto* avatar : m_avatars) {
        avatar->deflate();
      }
      handleDeflation();
    }
  });
}

void Player::handleAnnihilation()
{
  for (auto* avatar : m_avatars) {
    avatar->annihilate();
  }
  m_avatars.clear();
  m_status.isAlive = false;
  m_deflationTimer.cancel();
  m_annihilationEmitter.emit();
}

void Player::startMotion()
{
  for (auto* avatar : m_avatars) {
    avatar->startMotion();
  }
}

bool operator<(const Player& l, const Player& r)
{
  return std::tie(l.m_mass, r.m_name, r.m_id) < std::tie(r.m_mass, l.m_name, l.m_id);
}