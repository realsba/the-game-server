// file      : src/Config.hpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_CONFIG_HPP
#define THEGAME_CONFIG_HPP

#include "TimePoint.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <chrono>

namespace config {

struct Server {
  boost::asio::ip::tcp::endpoint address;
  uint32_t numThreads {0};
};

struct MySql {
  std::string database;
  std::string host;
  std::string user;
  std::string password;
  std::string charset;
  uint        port {0};
  uint        maxIdleTime {0};
};

struct InfluxDb {
  std::string host;
  std::string path;
  std::string token;
  Duration    interval;
  uint16_t    port {0};
  bool        enabled {false};
};

struct Leaderboard {
  Duration    updateInterval;
  uint32_t    limit {0};
};

struct Player {
  uint32_t  mass {0};
  uint32_t  maxCells {0};
  Duration  deflationThreshold;
  Duration  deflationInterval;
  float     deflationRatio {0};
  Duration  annihilationThreshold;
  float     pointerForceRatio {0};
};

struct Bot {
  uint32_t mass {0};
  Duration respawnDelay;
};

struct Avatar {
  uint32_t  minVelocity {0};
  uint32_t  maxVelocity {0};
  uint32_t  explosionMinMass {0};
  uint32_t  explosionParts {0};
  uint32_t  splitMinMass {0};
  uint32_t  splitVelocity {0};
  uint32_t  ejectionMinMass {0};
  uint32_t  ejectionVelocity {0};
  uint32_t  ejectionMass {0};
  uint32_t  ejectionMassLoss {0};
  uint32_t  recombinationTime {0}; // TODO: use Duration
};

struct Food {
  uint32_t  mass {0};
  uint32_t  radius {0};
  uint32_t  quantity {0};
  uint32_t  maxQuantity {0};
  uint32_t  minVelocity {0};
  uint32_t  maxVelocity {0};
  float     resistanceRatio {0};
  int       minColorIndex {0};
  int       maxColorIndex {0};
};

struct Virus {
  uint32_t  mass {0};
  uint32_t  quantity {0};
  uint32_t  maxQuantity {0};
  Duration  lifeTime;
  uint32_t  color {0};
};

using Phage = Virus;

struct Mother {
  uint32_t  mass {0};
  uint32_t  quantity {0};
  uint32_t  maxMass {0};
  uint32_t  maxQuantity {0};
  Duration  lifeTime;
  uint32_t  color {0};
  uint32_t  checkRadius {0};
  float     baseFoodProduction {0};
  uint32_t  nearbyFoodLimit {0};
  Duration  foodGenerationInterval;
};

struct Generator {
  struct Item {
    Duration  interval;
    uint32_t  quantity {0};
    bool      enabled {false};
  };

  Item food;
  Item virus;
  Item phage;
  Item mother;
};

struct Room {
  using BotNames = std::vector<std::string>;

  Leaderboard leaderboard;
  Player    player;
  Bot       bot;
  Avatar    avatar;
  Food      food;
  Virus     virus;
  Phage     phage;
  Mother    mother;
  Generator generator;

  float     eps {0.01};

  uint32_t  numThreads {0};
  Duration  updateInterval;

  uint32_t  spawnPosTryCount {0};

  Duration  checkPlayersInterval;
  Duration  destroyOutdatedCellsInterval;
  Duration  checkMothersInterval;
  Duration  produceMothersInterval;

  uint32_t  viewportBase {0};           // shorter side (height)
  float     viewportBuffer {0};         // buffer on each side
  float     aspectRatio {0};

  uint32_t  width {0};
  uint32_t  height {0};
  uint32_t  maxMass {0};
  uint32_t  maxPlayers {0};
  uint32_t  maxRadius {0};
  float     scaleRatio {0};
  uint32_t  explodeVelocity {0};

  BotNames  botNames;

  float     resistanceRatio {0};
  float     elasticityRatio {0};

  uint32_t  cellMinMass {0};
  float     cellRadiusRatio {0};

  double    simulationInterval {0};
  double    cellMinRadius {0};
  double    cellMaxRadius {0};
  double    cellRadiusDiff {0};
  double    avatarVelocityDiff {0};
};

class Config {
public:
  void load(const std::string& filename);

  Server    server;
  MySql     mysql;
  InfluxDb  influxdb;
  Room      room;
};

} // namespace config

#endif /* THEGAME_CONFIG_HPP */
