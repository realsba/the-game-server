// file   : src/Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PLAYER_HPP
#define THEGAME_PLAYER_HPP

#include "geometry/Vec2D.hpp"
#include "geometry/AABB.hpp"

#include "TimePoint.hpp"
#include "Gridmap.hpp"
#include "Event.hpp"

#include "types.hpp"

#include <boost/asio/steady_timer.hpp>

#include <string>
#include <vector>
#include <set>

namespace asio = boost::asio;

class Avatar;
class Room;

using Avatars = std::set<Avatar*>;

class Player {
public:
  Player(const asio::any_io_executor& executor, uint32_t id, Room& room, Gridmap& gridmap);
  virtual ~Player() = default;

  [[nodiscard]] uint32_t getId() const;
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] uint32_t getMass() const;
  [[nodiscard]] uint32_t getMaxMass() const;
  [[nodiscard]] const AABB& getViewBox() const;
  [[nodiscard]] Vec2D getPosition() const;
  [[nodiscard]] const Avatars& getAvatars() const;
  [[nodiscard]] Avatar* findTheBiggestAvatar() const;
  [[nodiscard]] Player* getKiller() const;
  [[nodiscard]] bool isDead() const;
  [[nodiscard]] uint8_t getStatus() const;
  [[nodiscard]] const Sessions& getSessions() const;

  virtual void init();

  void setName(const std::string& name);
  void setPointerOffset(const Vec2D& value);
  void setMainSession(const SessionPtr& sess);
  void addSession(const SessionPtr& sess);
  void removeSession(const SessionPtr& sess);
  void clearSessions();
  void setTargetPlayer(Player* player);
  virtual void addAvatar(Avatar* avatar);
  virtual void removeAvatar(Avatar* avatar, Player* killer);
  void eject(const Vec2D& point);
  void split(const Vec2D& point);
  void synchronize(uint32_t tick, const std::set<Cell*>& modified, const std::vector<uint32_t>& removed);
  void wakeUp();
  void calcParams();
  void applyPointerForce(uint32_t tick);
  void recombine(uint32_t tick);

  void subscribeToAnnihilationEvent(void* tag, Event<>::Handler&& handler);
  void unsubscribeFromAnnihilationEvent(void* tag);

protected:
  void recombine(Avatar& initiator, Avatar& target);
  void scheduleDeflation();
  void scheduleAnnihilation();
  void handleDeflation();
  void handleAnnihilation();
  void startMotion();

protected:
  struct Status {
    bool isOnline : 1 {false};
    bool isAlive : 1 {false};
  };

  asio::steady_timer    m_deflationTimer;
  asio::steady_timer    m_annihilationTimer;
  Event<>               m_annihilationEvent;

  const uint32_t        m_id {0};
  const Room&           m_room;
  const Gridmap&        m_gridmap;

  std::string           m_name;
  Sessions              m_sessions;
  SessionPtr            m_mainSession {nullptr};
  Avatars               m_avatars;
  std::set<uint32_t>    m_visibleIds;
  std::set<Sector*>     m_sectors;
  AABB                  m_viewport;
  AABB                  m_viewbox;
  Vec2D                 m_position;
  Vec2D                 m_pointerOffset;
  Sector*               m_leftTopSector {nullptr};
  Sector*               m_rightBottomSector {nullptr};
  Player*               m_targetPlayer {nullptr};
  Player*               m_killer {nullptr};
  uint32_t              m_mass {0};
  uint32_t              m_maxMass {0};
  Status                m_status;
  float                 m_scale {0};

  friend bool operator<(const Player& l, const Player& r);
};

bool operator<(const Player& l, const Player& r);

#endif /* THEGAME_PLAYER_HPP */
