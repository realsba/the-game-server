// file   : src/Room.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_HPP
#define THEGAME_ROOM_HPP

#include "PlayerFwd.hpp"

#include "IEntityFactory.hpp"

#include "ChatMessage.hpp"
#include "Gridmap.hpp"
#include "NextId.hpp"
#include "Config.hpp"
#include "Timer.hpp"
#include "types.hpp"

#include <utility>
#include <random>
#include <vector>
#include <list>
#include <map>
#include <set>

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
  std::random_device& randomGenerator() override;
  Vec2D getRandomPosition(double radius) const override;
  Vec2D getRandomDirection() const override;
  Gridmap& getGridmap() override;

  void doJoin(const SessionPtr& sess, uint32_t playerId);
  void doLeave(const SessionPtr& sess);
  void doPlay(const SessionPtr& sess, const std::string& name, uint8_t color);
  void doSpectate(const SessionPtr& sess, uint32_t targetId);
  void doMove(const SessionPtr& sess, const Vec2D& point);
  void doEject(const SessionPtr& sess, const Vec2D& point);
  void doSplit(const SessionPtr& sess, const Vec2D& point);
  void doWatch(const SessionPtr& sess, uint32_t playerId);
  void doChatMessage(const SessionPtr& sess, const std::string& text);

  Avatar& createAvatar() override;
  Food& createFood() override;
  Bullet& createBullet() override;
  Virus& createVirus() override;
  Phage& createPhage() override;
  Mother& createMother() override;

  void recalculateFreeSpace();
  void updateNewCellRegistries(Cell* cell, bool checkRandomPos = true);
  void removeCell(Cell* cell);
  void resolveCellPosition(Cell& cell);
  void destroyOutdatedCells(); // TODO: move logic to target classes
  void handlePlayerRequests();
  void removeDeadAvatars();
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
  void generateFood(uint32_t count);
  void generateViruses(uint32_t count);
  void generatePhages(uint32_t count);
  void generateMothers(uint32_t count);

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
  using RequestsMap = std::map<SessionPtr, Vec2D>;
  using Players = std::map<uint32_t, PlayerPtr>;
  using Bots = std::set<std::shared_ptr<Bot>>;

  mutable std::random_device  m_generator;
  asio::any_io_executor       m_executor;
  Timer                       m_updateTimer;
  Timer                       m_syncTimer;
  Timer                       m_updateLeaderboardTimer;
  Timer                       m_destroyOutdatedCellsTimer;
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
  std::set<PlayerPtr>         m_fighters;
  std::vector<PlayerPtr>      m_leaderboard;
  Bots                        m_bots;
  RequestsMap                 m_moveRequests;
  RequestsMap                 m_ejectRequests;
  RequestsMap                 m_splitRequests;
  NextId                      m_cellNextId;
  std::set<Avatar*>           m_avatarContainer;
  std::set<Food*>             m_foodContainer;
  std::set<Bullet*>           m_bulletContainer;
  std::set<Virus*>            m_virusContainer;
  std::set<Phage*>            m_phageContainer;
  std::set<Mother*>           m_motherContainer;
  std::vector<uint32_t>       m_removedCellIds;
  std::vector<Cell*>          m_createdCells;
  std::set<Cell*>             m_modifiedCells;
  std::set<Cell*>             m_activatedCells;
  std::set<Cell*>             m_processingCells;
  std::set<Cell*>             m_forCheckRandomPos;
  std::vector<Avatar*>        m_deadAvatars;
  std::list<ChatMessage>      m_chatHistory;

  TimePoint                   m_lastUpdate {TimePoint::clock::now()};
  double                      m_mass {0};
  const uint32_t              m_id {0};
  bool                        m_updateLeaderboard {false};
  bool                        m_hasFreeSpace {true};
};

#endif /* THEGAME_ROOM_HPP */
