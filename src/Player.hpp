// file   : Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "geometry/Vec2D.hpp"
#include "geometry/AABB.hpp"
#include "TimePoint.hpp"
#include "Gridmap.hpp"
#include "types.hpp"

#include <string>
#include <vector>
#include <set>

class WebsocketServer;
class Avatar;
class Room;

typedef std::vector<Avatar*> Avatars;

class Player {
public:
  Player(uint32_t id, WebsocketServer& wss, Room& room, Gridmap& gridmap);

  uint32_t getId() const;
  uint32_t getMass() const;
  uint32_t getMaxMass() const;
  const AABB& getViewBox() const;
  Vec2D getPosition() const;
  Vec2D getDestination() const;
  const Avatars& getAvatars() const;
  bool isDead() const;
  TimePoint getLastActivity() const;
  uint8_t getStatus() const;

  void init();
  void setPointer(const Vec2D& value);
  void addConnection(const ConnectionHdl& hdl);
  void removeConnection(const ConnectionHdl& hdl);
  void clearConnections();
  const Connections& getConnections() const;
  void addAvatar(Avatar* avatar);
  void removeAvatar(Avatar* avatar);
  void synchronize(uint32_t tick, const std::set<Cell*>& modified, const std::vector<uint32_t>& removed);
  void wakeUp();
  void calcParams();
  void removePlayer(Player* player);

  ConnectionHdl conn;
  std::string name;
  Player*     arrowPlayer {nullptr};
  Player*     killer {nullptr};
  bool        online {false};

private:
  WebsocketServer&      m_websocketServer;
  Room&                 m_room;
  Gridmap&              m_gridmap;
  Connections           m_connections;
  Avatars               m_avatars; // TODO: можна замінти на std::set
  std::set<uint32_t>    m_visibleIds;
  std::set<Sector*>     m_sectors;
  std::set<Cell*>       m_syncCells;
  std::set<uint32_t>    m_removedIds;
  AABB                  m_viewport;
  AABB                  m_viewbox;
  Vec2D                 m_position;
  Vec2D                 m_pointer;
  TimePoint             m_lastActivity {TimePoint::clock::now()};
  Sector*               m_leftTopSector {nullptr};
  Sector*               m_rightBottomSector {nullptr};
  uint32_t              m_id {0};
  uint32_t              m_mass {0};
  uint32_t              m_maxMass {0};
  float                 m_scale {0};

  friend bool operator<(const Player& l, const Player& r);
};

bool operator<(const Player& l, const Player& r);

#endif /* PLAYER_HPP */
