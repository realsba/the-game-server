// file   : src/Room.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_HPP
#define THEGAME_ROOM_HPP

#include "Gridmap.hpp"
#include "NextId.hpp"
#include "Config.hpp"
#include "Timer.hpp"
#include "types.hpp"

#include "entity/Avatar.hpp"

#include <utility>
#include <random>
#include <vector>
#include <list>
#include <map>
#include <set>

namespace asio = boost::asio;

class Player;
class Bot;
class Avatar;
class Food;
class Bullet;
class Virus;
class Phage;
class Mother;

struct ChatMessage {
  ChatMessage(uint32_t authorId, std::string author, std::string text)
    : text(std::move(text))
    , author(std::move(author))
    , authorId(authorId)
  {
  }

  std::string text;
  std::string author;
  uint32_t authorId;
};

class Room {
public:
  Room(asio::any_io_executor executor, uint32_t id);
  ~Room();

  const asio::any_io_executor& getExecutor() const;
  void init(const RoomConfig& config);

  void start();
  void stop();

  uint32_t getId() const;
  bool hasFreeSpace() const;
  const RoomConfig& getConfig() const;

  void join(const SessionPtr& sess);
  void leave(const SessionPtr& sess);
  void play(const SessionPtr& sess, const std::string& name, uint8_t color);
  void spectate(const SessionPtr& sess, uint32_t targetId);
  void point(const SessionPtr& sess, const Vec2D& point);
  void eject(const SessionPtr& sess, const Vec2D& point);
  void split(const SessionPtr& sess, const Vec2D& point);
  void watch(const SessionPtr& sess, uint32_t playerId);
  void chatMessage(const SessionPtr& sess, const std::string& text);

private:
  void doJoin(const SessionPtr& sess);
  void doLeave(const SessionPtr& sess);
  void doPlay(const SessionPtr& sess, const std::string& name, uint8_t color);
  void doSpectate(const SessionPtr& sess, uint32_t targetId);
  void doPoint(const SessionPtr& sess, const Vec2D& point);
  void doEject(const SessionPtr& sess, const Vec2D& point);
  void doSplit(const SessionPtr& sess, const Vec2D& point);
  void doWatch(const SessionPtr& sess, uint32_t playerId);
  void doChatMessage(const SessionPtr& sess, const std::string& text);

  void attract(Avatar& initiator, Avatar& target);
  void attract(Avatar& initiator, Food& target);
  void attract(Avatar& initiator, Bullet& target);
  void attract(Avatar& initiator, Virus& target);
  void attract(Avatar& initiator, Phage& target);
  void attract(Avatar& initiator, Mother& target);
  void attract(Avatar& initiator, const Vec2D& point);

  void update();

  Vec2D getRandomPosition(uint32_t radius) const;
  Avatar& createAvatar();
  Food& createFood();
  Bullet& createBullet();
  Virus& createVirus();
  Phage& createPhage();
  Mother& createMother();

  void recalculateFreeSpace();
  void updateNewCellRegistries(Cell* cell, bool checkRandomPos = true);
  void removeCell(Cell* cell);

  bool eject(Avatar& avatar, const Vec2D& point);
  bool split(Avatar& avatar, const Vec2D& point);
  void explode(Avatar& avatar);
  void explode(Mother& mother);

  void resolveCellPosition(Cell& cell);
  void destroyOutdatedCells();
  void handlePlayerRequests();
  void simulate(double dt);
  void synchronize();
  void updateLeaderboard();
  void checkMothers();
  void produceMothers();
  void checkPlayers();

  void generateFood();
  void generateViruses();
  void generatePhages();
  void generateMothers();

  void spawnFood(uint32_t count);
  void spawnViruses(uint32_t count);
  void spawnPhages(uint32_t count);
  void spawnMothers(uint32_t count);
  void spawnBot(uint32_t id, const std::string& name = "");

  void send(const BufferPtr& buffer);
  void sendPacketPlayer(const Player& player);
  void sendPacketPlayerRemove(uint32_t playerId);
  void sendPacketPlayerJoin(uint32_t playerId);
  void sendPacketPlayerLeave(uint32_t playerId);
  void sendPacketPlayerBorn(uint32_t playerId);
  void sendPacketPlayerDead(uint32_t playerId);

  void onAvatarDeath(Cell* cell);
  void onFoodDeath(Cell* cell);
  void onBulletDeath(Cell* cell);
  void onVirusDeath(Cell* cell);
  void onPhageDeath(Cell* cell);
  void onMotherDeath(Cell* cell);
  void onMotionStarted(Cell* cell);
  void onCellMassChange(Cell* cell, float deltaMass);
  void onAvatarMassChange(Cell* cell, float deltaMass);

private:
  friend class Avatar;
  friend class Food;
  friend class Bullet;
  friend class Virus;
  friend class Phage;
  friend class Mother;

  using RequestsMap = std::map<SessionPtr, Vec2D>;

  asio::any_io_executor       m_executor;
  Timer                       m_updateTimer;
  Timer                       m_checkPlayersTimer;
  Timer                       m_updateLeaderboardTimer;
  Timer                       m_destroyOutdatedCellsTimer;
  Timer                       m_checkMothersTimer;
  Timer                       m_produceMothersTimer;
  Timer                       m_foodGeneratorTimer;
  Timer                       m_virusGeneratorTimer;
  Timer                       m_phageGeneratorTimer;
  Timer                       m_motherGeneratorTimer;

  mutable std::random_device  m_generator;

  const uint32_t              m_id {0};
  RoomConfig                  m_config;
  Gridmap                     m_gridmap;
  Sessions                    m_sessions;
  std::map<uint32_t, Player*> m_players;
  std::set<Player*>           m_fighters;
  std::set<Player*>           m_zombiePlayers;
  std::vector<Player*>        m_leaderboard;
  std::set<Bot*>              m_bots;
  RequestsMap                 m_pointerRequests;
  RequestsMap                 m_ejectRequests;
  RequestsMap                 m_splitRequests;
  NextId                      m_cellNextId;
  std::set<Avatar*>           m_avatarContainer;
  std::set<Food*>             m_foodContainer;
  std::set<Bullet*>           m_bulletContainer;
  std::set<Virus*>            m_virusContainer;
  std::set<Phage*>            m_phageContainer;
  std::set<Mother*>           m_motherContainer;
  std::set<Cell*>             m_cells;
  std::vector<Cell*>          m_zombieAvatars;
  std::vector<Cell*>          m_zombieFoods;
  std::vector<Cell*>          m_zombieBullets;
  std::vector<Cell*>          m_zombieViruses;
  std::vector<Cell*>          m_zombiePhages;
  std::vector<Cell*>          m_zombieMothers;
  std::vector<uint32_t>       m_removedCellIds;
  std::vector<Cell*>          m_createdCells;
  std::set<Cell*>             m_modifiedCells;
  std::set<Cell*>             m_activatedCells;
  std::set<Cell*>             m_processingCells;
  std::set<Cell*>             m_forCheckRandomPos;
  std::list<ChatMessage>      m_chatHistory;

  TimePoint m_lastUpdate {TimePoint::clock::now()};

  uint32_t  m_tick {0};                        // TODO: stop using and remove
  double    m_mass {0};
  double    m_cellMinRadius {0};
  double    m_cellMaxRadius {0};
  double    m_cellRadiusDiff {0};
  double    m_avatarSpeedDiff {0};
  bool      m_updateLeaderboard {false};
  bool      m_hasFreeSpace {true};
};

#endif /* THEGAME_ROOM_HPP */
