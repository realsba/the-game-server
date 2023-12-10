// file   : Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Player.hpp"

#include "WebsocketServer.hpp"
#include "MemoryStream.hpp"
#include "Room.hpp"
#include "entity/Cell.hpp"
#include "entity/Avatar.hpp"
#include "packet/Packet.hpp"
#include "packet/OutputPacketTypes.hpp"

#include <algorithm>
#include <tuple>

Player::Player(uint32_t id, WebsocketServer& wss, Room& room, Gridmap& gridmap) :
  m_websocketServer(wss),
  m_room(room),
  m_gridmap(gridmap),
  m_id(id)
{ }

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

void Player::addConnection(const ConnectionHdl& hdl)
{
  if (m_connections.emplace(hdl).second) {
    // TODO: дані відправляться ще раз попереднім з'єднанням, бажано переробити механізм
    m_leftTopSector = nullptr;
    m_rightBottomSector = nullptr;
    m_sectors.clear();
  }
}

void Player::removeConnection(const ConnectionHdl& hdl)
{
  m_connections.erase(hdl);
  // TODO: неявно визначаємо що гравцем не управляє власник
  if (m_connections.empty()) {
    m_pointer.zero();
  }
}

void Player::clearConnections()
{
  m_connections.clear();
}

const Connections& Player::getConnections() const
{
  return m_connections;
}

void Player::addAvatar(Avatar* avatar)
{
  avatar->player = this;
  m_avatars.emplace_back(avatar);
}

void Player::removeAvatar(Avatar* avatar)
{
  const auto& it = std::find(m_avatars.begin(), m_avatars.end(), avatar);
  if (it != m_avatars.end()) {
    m_avatars.erase(it);
  }
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

  if (m_connections.empty()) {
    return;
  }

  if (sectorsChanged) {
    const auto& sectors = m_gridmap.getSectors(viewport);
    for (auto it = m_sectors.begin(); it != m_sectors.end();) {
      if (sectors.find(*it) == sectors.end()) {
        const Sector* sector = *it;
        for (Cell* cell : sector->cells) {
          if (!cell->intersects(m_viewbox)) {
            m_removedIds.insert(cell->id);
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
        m_syncCells.insert(sector->cells.begin(), sector->cells.end());
      }
    }
  }
  for (Cell* cell : modified) {
    if (cell->player == this || cell->intersects(m_viewbox)) {
      m_syncCells.insert(cell);
    }
  }
  for (auto id : removed) {
    if (m_visibleIds.erase(id)) {
      m_removedIds.insert(id);
    }
  }

  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  Packet packet;
  packet.prepareHeader(ms);
  ms.writeUInt32(tick);
  ms.writeFloat(m_scale);
  ms.writeUInt16(m_syncCells.size());
  for (Cell* cell : m_syncCells) {
    cell->format(ms);
    m_visibleIds.insert(cell->id);
  }
  ms.writeUInt16(m_removedIds.size());
  for (auto id : m_removedIds) {
    ms.writeUInt32(id);
  }
  ms.writeInt8(m_avatars.size());
  for (const Avatar* avatar : m_avatars) {
    ms.writeUInt32(avatar->id);
    ms.writeFloat(avatar->maxSpeed);
    ms.writeUInt32(avatar->protection);
  }
  if (arrowPlayer) {
    ms.writeInt32(arrowPlayer->getId());
    const auto& position = arrowPlayer->getPosition();
    ms.writeFloat(position.x);
    ms.writeFloat(position.y);
  } else {
    ms.writeInt32(0);
  }
  packet.writeHeader(ms, OutputPacketTypes::Frame);
  for (auto&& hdl : m_connections) {
    m_websocketServer.send(hdl, ms);
  }

  m_syncCells.clear();
  m_removedIds.clear();
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
  for (const Avatar* avatar : m_avatars) {
    position += avatar->position * avatar->mass;
    m_mass += avatar->mass;
    if (avatar->radius > radius) {
      radius = avatar->radius;
    }
  }
  if (m_mass != 0) {
    position /= m_mass;
  }
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