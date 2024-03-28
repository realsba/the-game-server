// file   : src/Room.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_HPP
#define THEGAME_ROOM_HPP

#include "PlayerFwd.hpp"

#include "IEntityFactory.hpp"

#include "ChatMessage.hpp"
#include "Config.hpp"
#include "Gridmap.hpp"
#include "NextId.hpp"
#include "Timer.hpp"
#include "types.hpp"

#include <list>
#include <unordered_map>
#include <random>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/asio/strand.hpp>

namespace asio = boost::asio;

class Bot;

class Room : public IEntityFactory {
public:
  Room(asio::any_io_executor executor, uint32_t id);
  ~Room() override;

  void init(const config::Room& config);

  void start();
  void stop();

  bool hasFreeSpace() const;

  void join(const SessionPtr& sess, uint32_t playerId);
  void leave(const SessionPtr& sess);
  void play(const SessionPtr& sess, const std::string& name, uint8_t color);
  void spectate(const SessionPtr& sess, uint32_t targetId);
  void move(const SessionPtr& sess, const Vec2D& point);
  void eject(const SessionPtr& sess, const Vec2D& point);
  void split(const SessionPtr& sess, const Vec2D& point);
  void watch(const SessionPtr& sess, uint32_t playerId);
  void chatMessage(const SessionPtr& sess, const std::string& text);

private:
  Avatar& createAvatar() override;
  Food& createFood() override;
  Bullet& createBullet() override;
  Virus& createVirus() override;
  Phage& createPhage() override;
  Mother& createMother() override;
  std::random_device& randomGenerator() override;
  Vec2D getRandomPosition(double radius) const override;
  Vec2D getRandomDirection() const override;
  Gridmap& getGridmap() override;
  asio::any_io_executor& getGameExecutor() override;
  asio::any_io_executor& getDeathExecutor() override;

  void doJoin(const SessionPtr& sess, uint32_t playerId);
  void doLeave(const SessionPtr& sess);
  void doPlay(const SessionPtr& sess, const std::string& name, uint8_t color);
  void doSpectate(const SessionPtr& sess, uint32_t targetId);
  void doMove(const SessionPtr& sess, const Vec2D& point);
  void doEject(const SessionPtr& sess, const Vec2D& point);
  void doSplit(const SessionPtr& sess, const Vec2D& point);
  void doWatch(const SessionPtr& sess, uint32_t playerId);
  void doChatMessage(const SessionPtr& sess, const std::string& text);

  void recalculateFreeSpace();
  void updateNewCellRegistries(Cell* cell, bool checkRandomPos = true);
  void prepareCellForDestruction(Cell* cell);
  void removeCell(Cell* cell);
  void resolveCellPosition(Cell& cell);
  void killExpiredCells(); // TODO: move logic to target classes
  void handlePlayerRequests();
  void update();
  void synchronize();
  void updateLeaderboard();
  void removeFromLeaderboard(const PlayerPtr& player);
  void updateNearbyFoodForMothers();
  void generateFoodByMothers();
  PlayerPtr createPlayer(uint32_t id, const std::string& name);
  void createBots();

  void generateFood();
  void generateViruses();
  void generatePhages();
  void generateMothers();
  void generateFood(int quantity);
  void generateViruses(int quantity);
  void generatePhages(int quantity);
  void generateMothers(int quantity);

  void serialize(Buffer& buffer);
  void send(const BufferPtr& buffer);
  void sendPacketPlayer(const Player& player);
  void sendPacketPlayerRemove(uint32_t playerId);
  void sendPacketPlayerJoin(uint32_t playerId);
  void sendPacketPlayerLeave(uint32_t playerId);
  void sendPacketPlayerBorn(uint32_t playerId);
  void sendPacketPlayerDead(uint32_t playerId);

  void onPlayerRespawn(const PlayerWPtr& weakPlayer);
  void onPlayerDeath(const PlayerWPtr& weakPlayer);
  void onPlayerAnnihilates(const PlayerWPtr& weakPlayer);
  void onAvatarDeath(Avatar* avatar);
  void onFoodDeath(Food* food);
  void onBulletDeath(Bullet* bullet);
  void onVirusDeath(Virus* virus);
  void onPhageDeath(Phage* phage);
  void onMotherDeath(Mother* mother);
  void onMotionStarted(Cell* cell);
  void onMotionStopped(Cell* cell);
  void onCellMassChange(Cell* cell, float deltaMass);
  void onAvatarMassChange(Avatar* avatar, float deltaMass);
  void onMotherMassChange(Mother* mother, float deltaMass);

private:
  using RequestsMap = std::unordered_map<SessionPtr, Vec2D>;
  using Players = std::unordered_map<uint32_t, PlayerPtr>;
  using Fighters = std::unordered_set<PlayerPtr>;
  using Bots = std::unordered_set<std::shared_ptr<Bot>>;

  mutable std::random_device  m_generator;
  asio::any_io_executor       m_executor;
  asio::io_context            m_gameContext;
  asio::io_context            m_deathContext;
  asio::any_io_executor       m_gameExecutor {asio::make_strand(m_gameContext)};
  asio::any_io_executor       m_deathExecutor {asio::make_strand(m_deathContext)};
  Timer                       m_updateTimer;
  Timer                       m_syncTimer;
  Timer                       m_updateLeaderboardTimer;
  Timer                       m_checkExpirableCellsTimer;
  Timer                       m_updateNearbyFoodForMothersTimer;
  Timer                       m_generateFoodByMothersTimer;
  Timer                       m_foodGeneratorTimer;
  Timer                       m_virusGeneratorTimer;
  Timer                       m_phageGeneratorTimer;
  Timer                       m_motherGeneratorTimer;

  config::Room                m_config;
  Gridmap                     m_gridmap;
  Sessions                    m_sessions;
  Players                     m_players;
  Fighters                    m_fighters;
  std::vector<PlayerPtr>      m_leaderboard;
  Bots                        m_bots;
  RequestsMap                 m_moveRequests;
  RequestsMap                 m_ejectRequests;
  RequestsMap                 m_splitRequests;
  NextId                      m_cellNextId;
  std::unordered_set<Cell*>   m_cells;
  std::unordered_set<Virus*>  m_viruses;
  std::unordered_set<Phage*>  m_phages;
  std::unordered_set<Mother*> m_mothers;
  std::unordered_set<Cell*>   m_modifiedCells;
  std::unordered_set<Cell*>   m_activatedCells;
  std::unordered_set<Cell*>   m_processingCells;
  std::unordered_set<Cell*>   m_forCheckRandomPos;
  std::unordered_set<Cell*>   m_createdCells;
  std::vector<Cell*>          m_deadCells;
  std::list<ChatMessage>      m_chatHistory;
  int                         m_foodQuantity {0};
  int                         m_virusesQuantity {0};
  int                         m_phagesQuantity {0};
  int                         m_mothersQuantity {0};

  TimePoint                   m_lastUpdate {TimePoint::clock::now()};
  double                      m_mass {0};
  const uint32_t              m_id {0};
  bool                        m_updateLeaderboard {false};
  bool                        m_hasFreeSpace {true};
};

#endif /* THEGAME_ROOM_HPP */
