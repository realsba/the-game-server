// file      : Config.hpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_CONFIG_HPP
#define THEGAME_CONFIG_HPP

#include "TimePoint.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <chrono>

namespace asio = boost::asio;

struct MySQLConfig {
  std::string database;
  std::string server;
  std::string user;
  std::string password;
  std::string charset;
  uint        port {0};
  uint        maxIdleTime {0};
};

struct RoomConfig {
  float eps {0.01};

  Duration  updateInterval;

  uint32_t  simulationsPerUpdate {0};
  uint32_t  spawnPosTryCount {0};

  Duration  checkPlayersInterval {};
  Duration  updateLeaderboardInterval {};
  Duration  destroyOutdatedCellsInterval {};
  Duration  checkMothersInterval {};
  Duration  produceMothersInterval {};

  uint32_t  viewportBase {0};           // коротша сторона (висота)
  float     viewportBuffer {0};         // запас в кожну сторону
  float     aspectRatio {0};

  uint32_t  width {0};
  uint32_t  height {0};
  uint32_t  maxMass {0};
  uint32_t  maxPlayers {0};
  uint32_t  maxRadius {0};
  uint32_t  leaderboardVisibleItems {0};
  float     scaleRatio {0};
  uint32_t  explodeImpulse {0};

  uint32_t  playerMaxCells {0};
  Duration  playerDeflationInterval {};
  float     playerDeflationRatio {0};
  Duration  playerAnnihilationInterval {};
  float     playerForceRatio {0};

  uint32_t  botAmount {0};
  uint32_t  botStartMass {0};
  float     botForceCornerRatio {0};
  float     botForceFoodRatio {0};
  float     botForceHungerRatio {0};
  float     botForceDangerRatio {0};
  float     botForceStarRatio {0};

  float     resistanceRatio {0};        // коефіцієнт лобового опору
  float     elasticityRatio {0};        // коефіцієнт відштовхування

  uint32_t  cellMinMass {0};
  float     cellRadiusRatio {0};

  uint32_t  avatarStartMass {0};
  uint32_t  avatarMinSpeed {0};
  uint32_t  avatarMaxSpeed {0};
  uint32_t  avatarExplodeMinMass {0};
  uint32_t  avatarExplodeParts {0};
  uint32_t  avatarSplitMinMass {0};
  uint32_t  avatarSplitImpulse {0};
  uint32_t  avatarEjectMinMass {0};
  uint32_t  avatarEjectImpulse {0};
  uint32_t  avatarRecombineTime {0}; // TODO: use Duration
  uint32_t  avatarEjectMass {0};
  uint32_t  avatarEjectMassLoss {0};

  uint32_t  foodStartAmount {0};
  uint32_t  foodMaxAmount {0};
  uint32_t  foodMass {0};
  uint32_t  foodRadius {0};
  uint32_t  foodMinImpulse {0};
  uint32_t  foodMaxImpulse {0};
  float     foodResistanceRatio {0};

  uint32_t  virusStartMass {0};
  uint32_t  virusStartAmount {0};
  uint32_t  virusMaxAmount {0};
  Duration  virusLifeTime {};
  uint32_t  virusColor {0};

  uint32_t  phageStartMass {0};
  uint32_t  phageStartAmount {0};
  uint32_t  phageMaxAmount {0};
  Duration  phageLifeTime {};
  uint32_t  phageColor {0};

  uint32_t  motherStartMass {0};
  uint32_t  motherStartAmount {0};
  uint32_t  motherMaxAmount {0};
  Duration  motherLifeTime {};
  uint32_t  motherExplodeMass {0};
  uint32_t  motherColor {0};
  uint32_t  motherCheckRadius {0};

  float     spawnFoodMass {0};
  float     spawnVirusMass {0};
  float     spawnPhageMass {0};
  float     spawnMotherMass {0};
};

class Config {
public:
  void load(const std::string& filename);

  asio::ip::tcp::endpoint     address;
  std::chrono::milliseconds   updateInterval {std::chrono::seconds(1)};
  std::chrono::seconds        statisticInterval {std::chrono::minutes(1)};
  uint                        ioContextThreads {1};
  uint                        roomThreads {2};

  std::string                 influxdbServer;
  uint                        influxdbPort {0};

  MySQLConfig                 mysql;
  RoomConfig                  room;
};

#endif /* THEGAME_CONFIG_HPP */
