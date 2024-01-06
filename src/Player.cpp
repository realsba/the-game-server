// file   : Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Player.hpp"

#include "Session.hpp"
#include "Room.hpp"

#include "entity/Cell.hpp"
#include "entity/Avatar.hpp"
#include "src/packet/serialization.hpp"
#include "packet/OutputPacketTypes.hpp"

#include <algorithm>
#include <tuple>

Player::Player(uint32_t id, Room& room, Gridmap& gridmap) :
  m_room(room),
  m_gridmap(gridmap),
  m_id(id)
{
}

uint32_t Player::getId() const
{
  return m_id;
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

Vec2D Player::getDestination() const
{
  return m_position + m_pointer;
}

const Avatars& Player::getAvatars() const
{
  return m_avatars;
}

bool Player::isDead() const
{
  return m_avatars.empty();
}

TimePoint Player::getLastActivity() const
{
  return m_lastActivity;
}

uint8_t Player::getStatus() const
{
  return (online ? 1 : 0) | (m_avatars.empty() ? 0 : 2);
}

void Player::init()
{
  m_maxMass = 0;
}

void Player::setPointer(const Vec2D& value)
{
  m_pointer = value;
}

void Player::addConnection(const SessionPtr& sess)
{
  if (m_sessions.emplace(sess).second) {
    m_leftTopSector = nullptr;
    m_rightBottomSector = nullptr;
    m_sectors.clear();
    m_visibleIds.clear();
  }
}

void Player::removeConnection(const SessionPtr& sess)
{
  m_sessions.erase(sess);
  if (m_sessions.empty()) {
    m_pointer.zero();
  }
}

void Player::clearConnections()
{
  m_sessions.clear();
}

const Sessions& Player::getSessions() const
{
  return m_sessions;
}

void Player::addAvatar(Avatar* avatar)
{
  avatar->player = this;
  m_avatars.emplace(avatar);
}

void Player::removeAvatar(Avatar* avatar)
{
  m_avatars.erase(avatar);
}

void Player::synchronize(uint32_t tick, const std::set<Cell*>& modified, const std::vector<uint32_t>& removed)
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
    for (auto it = m_sectors.begin(); it != m_sectors.end();) {
      if (sectors.find(*it) == sectors.end()) {
        const Sector* sector = *it;
        for (Cell* cell : sector->cells) {
          if (!cell->intersects(m_viewbox)) {
            removedIds.insert(cell->id);
          }
        }
        it = m_sectors.erase(it);
      } else {
        ++it;
      }
    }
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
  serialize(*buffer, static_cast<uint8_t>(OutputPacketTypes::Frame));
  serialize(*buffer, tick);
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
    serialize(*buffer, avatar->maxSpeed);
    serialize(*buffer, avatar->protection);
  }
  if (arrowPlayer) {
    serialize(*buffer, arrowPlayer->getId());
    const auto& position = arrowPlayer->getPosition();
    serialize(*buffer, position.x);
    serialize(*buffer, position.y);
  } else {
    serialize(*buffer, static_cast<std::uint32_t>(0));
  }
  for (const auto& session : m_sessions) {
    session->send(buffer);
  }

  syncCells.clear();
  removedIds.clear();
}

void Player::wakeUp()
{
  m_lastActivity = TimePoint::clock::now();
}

void Player::calcParams()
{
  m_mass = 0;
  if (m_avatars.empty()) {
    return;
  }
  const auto& config = m_room.getConfig();
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
  float scale = radius > config.maxRadius ? 1 + config.scaleRatio * (radius / config.maxRadius - 1) : 1;
  if (m_scale != scale) {
    m_scale = scale;
    auto halfHeight = 0.5 * config.viewportBase * (1 + 2 * config.viewportBuffer) * m_scale;
    auto halfWidth = halfHeight * config.aspectRatio;
    Vec2D buffer(halfWidth, halfHeight);
    m_viewport.a = position - buffer;
    m_viewport.b = position + buffer;
  } else {
    m_viewport.translate(position - m_position);
  }
  m_position = position;
}

void Player::removePlayer(Player* player)
{
  if (arrowPlayer == player) {
    arrowPlayer = nullptr;
  }
}

bool operator<(const Player& l, const Player& r)
{
  return std::tie(l.m_mass, r.name, r.m_id) < std::tie(r.m_mass, l.name, l.m_id);
}