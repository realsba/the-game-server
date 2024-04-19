// file   : src/Player.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PLAYER_HPP
#define THEGAME_PLAYER_HPP

#include "EventEmitter.hpp"
#include "Gridmap.hpp"
#include "IEntityFactory.hpp"
#include "PlayerFwd.hpp"
#include "TimePoint.hpp"
#include "types.hpp"

#include "geometry/AABB.hpp"
#include "geometry/Vec2D.hpp"

#include <boost/asio/steady_timer.hpp>

#include <string>
#include <unordered_set>
#include <vector>

namespace asio = boost::asio;

namespace config {
  class Room;
}

class Avatar;

class Player : public std::enable_shared_from_this<Player> {
public:
  Player(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);
  virtual ~Player() = default;

  [[nodiscard]] uint32_t getId() const;
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] uint32_t getMass() const;
  [[nodiscard]] uint32_t getMaxMass() const;
  [[nodiscard]] const AABB& getViewBox() const;
  [[nodiscard]] Vec2D getPosition() const;
  [[nodiscard]] Avatar* findTheBiggestAvatar() const;
  [[nodiscard]] bool isDead() const;
  [[nodiscard]] uint8_t getStatus() const;

  virtual void respawn();

  void setName(const std::string& name);
  void setColor(uint8_t color);
  void setPointerOffset(const Vec2D& value);
  void setMainSession(const SessionPtr& sess);
  void addSession(const SessionPtr& sess);
  void removeSession(const SessionPtr& sess);
  void clearSessions();
  void setTargetPlayer(const PlayerPtr& player);
  void eject(const Vec2D& point);
  void split(const Vec2D& point);
  void synchronize(const std::unordered_set<Cell*>& modified, const std::vector<uint32_t>& removed);
  void wakeUp();
  void calcParams(); // TODO: optimize using
  void applyPointerForce();
  void recombine();
  void setKiller(const PlayerPtr& killer);

  void subscribeToAnnihilation(void* tag, EventEmitter<>::Handler&& handler);
  void subscribeToRespawn(void* tag, EventEmitter<>::Handler&& handler);
  void subscribeToDeath(void* tag, EventEmitter<>::Handler&& handler);

protected:
  virtual void addAvatar(Avatar* avatar);
  virtual void removeAvatar(Avatar* avatar);

  void recombine(Avatar& initiator, Avatar& target);
  void scheduleDeflation();
  void scheduleAnnihilation();
  void handleDeflation();
  void handleAnnihilation();
  void startMotion();
  void onAvatarExplode(Avatar* avatar);
  void onDeath();

protected:
  using Avatars = std::unordered_set<Avatar*>;
  using VisibleIds = std::unordered_set<uint32_t>;

  struct Status {
    bool isOnline : 1 {false};
    bool isAlive : 1 {false};
  };

  asio::steady_timer    m_deflationTimer;
  asio::steady_timer    m_annihilationTimer;
  EventEmitter<>        m_annihilationEmitter;
  EventEmitter<>        m_deathEmitter;
  EventEmitter<>        m_respawnEmitter;

  IEntityFactory&       m_entityFactory;
  const config::Room&   m_config;
  const Gridmap&        m_gridmap;
  const uint32_t        m_id {0};

  std::string           m_name;
  Sessions              m_sessions;
  SessionPtr            m_mainSession;
  Avatars               m_avatars;
  VisibleIds            m_visibleIds;
  std::set<Sector*>     m_sectors;
  AABB                  m_viewport;
  AABB                  m_viewbox;
  Vec2D                 m_position;
  Vec2D                 m_pointerOffset;
  Sector*               m_leftTopSector {nullptr};
  Sector*               m_rightBottomSector {nullptr};
  PlayerWPtr            m_targetPlayer;
  PlayerWPtr            m_killer;
  uint32_t              m_mass {0};
  uint32_t              m_maxMass {0};
  float                 m_scale {0};
  uint8_t               m_color {0};
  uint8_t               m_directionToTargetPlayer {0};
  Status                m_status;

  friend bool operator<(const Player& l, const Player& r);
};

bool operator<(const Player& l, const Player& r);

#endif /* THEGAME_PLAYER_HPP */
