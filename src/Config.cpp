// file      : Config.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include "Config.hpp"
#include "Logger.hpp"

#include <libconfig.h++>
#include <chrono>

using namespace boost;
using namespace std::chrono;

void lookupValue(const libconfig::Config& cfg, const std::string& path, std::chrono::milliseconds& to)
{
  std::string from;
  if (cfg.lookupValue(path, from)) {
    std::stringstream ss(from);
    //ss >> to;
  } else {
    LOG_WARN << "Bad " << path;
  }
}

void lookupValue(const libconfig::Config& cfg, const std::string& path, std::chrono::seconds& to)
{
  std::string from;
  if (cfg.lookupValue(path, from)) {
    std::stringstream ss(from);
    //ss >> to;
  } else {
    LOG_WARN << "Bad " << path;
  }
}

template<class T>
void lookupValue(const libconfig::Config& cfg, const std::string& path, T& value)
{
  if (!cfg.lookupValue(path, value)) {
    LOG_WARN << "Bad " << path << ", will be used " << value;
  }
}

Config::Config()
{
  room.updateInterval = std::chrono::milliseconds(10);
  room.tickInterval = std::chrono::milliseconds(50);
  room.simulationIterations = 5;
  room.spawnPosTryCount = 10;

  room.viewportBase = 743;
  room.aspectRatio = 16.0/9.0;

  room.width = 6000;
  room.height = 6000;
  room.maxMass = 100000;
  room.maxPlayers = 50;
  room.maxRadius = 100;
  room.leaderboardVisibleItems = 20;
  room.scaleRatio = 0.5;
  room.explodeImpulse = 500;

  room.playerMaxCells = 16;
  room.playerDeflationTime = 30;
  room.playerDeflationRatio = 0.01;
  room.playerAnnihilationTime = 60;
  room.playerForceRatio = 2.5;

  room.resistanceRatio = 750.0;
  room.elasticityRatio = 15.0;

  room.cellMinMass = 35;
  room.cellRadiusRatio = 6.0;

  room.avatarStartMass = 35;
  room.avatarMinSpeed = 200;
  room.avatarMaxSpeed = 600;
  room.avatarExplodeMinMass = 35;
  room.avatarExplodeParts = 5;
  room.avatarSplitMinMass = 100;
  room.avatarSplitImpulse = 600;
  room.avatarEjectMinMass = 82;
  room.avatarEjectImpulse = 550;
  room.avatarEjectMass = 35;
  room.avatarEjectMassLoss = 47;
  room.avatarRecombineTime = 8;

  room.foodMass = 1;
  room.foodRadius = 8;
  room.foodMinImpulse = 190;
  room.foodMaxImpulse = 210;
  room.foodResistanceRatio = 30.0;

  room.massImpulseRatio = 2.0;

  room.virusStartMass = 165;
  room.virusLifeTime = 300;
  room.virusColor = 13;

  room.phageStartMass = 165;
  room.phageLifeTime = 300;
  room.phageColor = 14;

  room.motherStartMass = 240;
  room.motherLifeTime = 600;
  room.motherExplodeMass = 3000;
  room.motherColor = 12;
  room.motherCheckRadius = 60;
}

bool Config::loadFromFile(const std::string& filename)
{
  libconfig::Config cfg;

  try {
    cfg.readFile(filename.c_str());

    std::string host;
    int port;
    lookupValue(cfg, "host", host);
    lookupValue(cfg, "port", port);
    address = asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port);
    lookupValue(cfg, "listenBacklog", listenBacklog);
    lookupValue(cfg, "ioServiceThreads", ioServiceThreads);
    if (ioServiceThreads < 1) {
      LOG_ERROR << "ioServiceThreads must be not less than 1";
      return false;
    }
    lookupValue(cfg, "roomThreads", roomThreads);
    if (ioServiceThreads < 1) {
      LOG_ERROR << "roomThreads must be not less than 1";
      return false;
    }
    lookupValue(cfg, "updateInterval", updateInterval);
    if (updateInterval == std::chrono::system_clock::duration::zero()) {
      // TODO: implement
      //LOG_WARN << "Bad updateInterval: " << updateInterval;
    }
    lookupValue(cfg, "statisticInterval", statisticInterval);
    if (statisticInterval == std::chrono::system_clock::duration::zero()) {
      // TODO: implement
      //LOG_WARN << "Bad statisticInterval: " << statisticInterval;
    }
    lookupValue(cfg, "connectionTTL", connectionTTL);
    if (connectionTTL == std::chrono::system_clock::duration::zero()) {
      // TODO: implement
      //LOG_WARN << "Bad connectionTTL: " << connectionTTL;
    }

    lookupValue(cfg, "influxdb.server", influxdbServer);
    lookupValue(cfg, "influxdb.port", influxdbPort);

    lookupValue(cfg, "mysql.database", mysql.database);
    lookupValue(cfg, "mysql.server", mysql.server);
    lookupValue(cfg, "mysql.user", mysql.user);
    lookupValue(cfg, "mysql.password", mysql.password);
    lookupValue(cfg, "mysql.charset", mysql.charset);
    lookupValue(cfg, "mysql.port", mysql.port);
    lookupValue(cfg, "mysql.maxIdleTime", mysql.maxIdleTime);

    lookupValue(cfg, "room.updateInterval", room.updateInterval);
    if (room.updateInterval == std::chrono::system_clock::duration::zero()) {
      // TODO: implement
      //LOG_WARN << "Bad room.updateInterval: " << room.updateInterval;
    }
    lookupValue(cfg, "room.tickInterval", room.tickInterval);
    if (room.tickInterval == std::chrono::system_clock::duration::zero()) {
      // TODO: implement
      //LOG_WARN << "Bad room.tickInterval: " << room.tickInterval;
    }
    lookupValue(cfg, "room.simulationIterations", room.simulationIterations);
    lookupValue(cfg, "room.spawnPosTryCount", room.spawnPosTryCount);
    lookupValue(cfg, "room.checkPlayers", room.checkPlayers);
    lookupValue(cfg, "room.updateLeaderboard", room.updateLeaderboard);
    lookupValue(cfg, "room.destroyOutdatedCells", room.destroyOutdatedCells);
    lookupValue(cfg, "room.checkMothers", room.checkMothers);
    lookupValue(cfg, "room.mothersProduce", room.mothersProduce);

    lookupValue(cfg, "room.viewportBase", room.viewportBase);
    lookupValue(cfg, "room.viewportBuffer", room.viewportBuffer);
    lookupValue(cfg, "room.aspectRatio", room.aspectRatio);

    lookupValue(cfg, "room.width", room.width);
    lookupValue(cfg, "room.height", room.height);
    lookupValue(cfg, "room.maxMass", room.maxMass);
    lookupValue(cfg, "room.maxPlayers", room.maxPlayers);
    lookupValue(cfg, "room.maxRadius", room.maxRadius);
    lookupValue(cfg, "room.leaderboardVisibleItems", room.leaderboardVisibleItems);
    lookupValue(cfg, "room.scaleRatio", room.scaleRatio);
    lookupValue(cfg, "room.explodeImpulse", room.explodeImpulse);

    lookupValue(cfg, "room.playerMaxCells", room.playerMaxCells);
    lookupValue(cfg, "room.playerDeflationTime", room.playerDeflationTime);
    lookupValue(cfg, "room.playerDeflationRatio", room.playerDeflationRatio);
    lookupValue(cfg, "room.playerAnnihilationTime", room.playerAnnihilationTime);
    lookupValue(cfg, "room.playerForceRatio", room.playerForceRatio);

    lookupValue(cfg, "room.botAmount", room.botAmount);
    lookupValue(cfg, "room.botStartMass", room.botStartMass);
    lookupValue(cfg, "room.botForceCornerRatio", room.botForceCornerRatio);
    lookupValue(cfg, "room.botForceFoodRatio", room.botForceFoodRatio);
    lookupValue(cfg, "room.botForceHungerRatio", room.botForceHungerRatio);
    lookupValue(cfg, "room.botForceDangerRatio", room.botForceDangerRatio);
    lookupValue(cfg, "room.botForceStarRatio", room.botForceStarRatio);

    lookupValue(cfg, "room.resistanceRatio", room.resistanceRatio);
    lookupValue(cfg, "room.elasticityRatio", room.elasticityRatio);

    lookupValue(cfg, "room.cellMinMass", room.cellMinMass);
    lookupValue(cfg, "room.cellRadiusRatio", room.cellRadiusRatio);

    lookupValue(cfg, "room.avatarStartMass", room.avatarStartMass);
    lookupValue(cfg, "room.avatarMinSpeed", room.avatarMinSpeed);
    lookupValue(cfg, "room.avatarMaxSpeed", room.avatarMaxSpeed);
    lookupValue(cfg, "room.avatarExplodeMinMass", room.avatarExplodeMinMass);
    lookupValue(cfg, "room.avatarExplodeParts", room.avatarExplodeParts);
    lookupValue(cfg, "room.avatarSplitMinMass", room.avatarSplitMinMass);
    lookupValue(cfg, "room.avatarSplitImpulse", room.avatarSplitImpulse);
    lookupValue(cfg, "room.avatarEjectMinMass", room.avatarEjectMinMass);
    lookupValue(cfg, "room.avatarEjectImpulse", room.avatarEjectImpulse);
    lookupValue(cfg, "room.avatarEjectMass", room.avatarEjectMass);
    lookupValue(cfg, "room.avatarEjectMassLoss", room.avatarEjectMassLoss);
    lookupValue(cfg, "room.avatarRecombineTime", room.avatarRecombineTime);

    lookupValue(cfg, "room.foodStartAmount", room.foodStartAmount);
    lookupValue(cfg, "room.foodMaxAmount", room.foodMaxAmount);
    lookupValue(cfg, "room.foodMass", room.foodMass);
    lookupValue(cfg, "room.foodRadius", room.foodRadius);
    lookupValue(cfg, "room.foodMinImpulse", room.foodMinImpulse);
    lookupValue(cfg, "room.foodMaxImpulse", room.foodMaxImpulse);
    lookupValue(cfg, "room.foodResistanceRatio", room.foodResistanceRatio);
    if (room.foodMinImpulse > room.foodMaxImpulse) {
      LOG_WARN << "room.foodMinImpulse > room.foodMaxImpulse";
    }

    lookupValue(cfg, "room.massImpulseRatio", room.massImpulseRatio);

    lookupValue(cfg, "room.virusStartMass", room.virusStartMass);
    lookupValue(cfg, "room.virusStartAmount", room.virusStartAmount);
    lookupValue(cfg, "room.virusMaxAmount", room.virusMaxAmount);
    lookupValue(cfg, "room.virusLifeTime", room.virusLifeTime);
    lookupValue(cfg, "room.virusColor", room.virusColor);

    lookupValue(cfg, "room.phageStartMass", room.phageStartMass);
    lookupValue(cfg, "room.phageStartAmount", room.phageStartAmount);
    lookupValue(cfg, "room.phageMaxAmount", room.phageMaxAmount);
    lookupValue(cfg, "room.phageLifeTime", room.phageLifeTime);
    lookupValue(cfg, "room.phageColor", room.phageColor);

    lookupValue(cfg, "room.motherStartMass", room.motherStartMass);
    lookupValue(cfg, "room.motherStartAmount", room.motherStartAmount);
    lookupValue(cfg, "room.motherMaxAmount", room.motherMaxAmount);
    lookupValue(cfg, "room.motherLifeTime", room.motherLifeTime);
    lookupValue(cfg, "room.motherExplodeMass", room.motherExplodeMass);
    lookupValue(cfg, "room.motherColor", room.motherColor);
    lookupValue(cfg, "room.motherCheckRadius", room.motherCheckRadius);

    lookupValue(cfg, "room.spawnFoodMass", room.spawnFoodMass);
    lookupValue(cfg, "room.spawnVirusMass", room.spawnVirusMass);
    lookupValue(cfg, "room.spawnPhageMass", room.spawnPhageMass);
    lookupValue(cfg, "room.spawnMotherMass", room.spawnMotherMass);
  } catch (const libconfig::FileIOException& e) {
    LOG_ERROR << "Can't read config file \"" << filename << "\"";
    return false;
  } catch (const libconfig::ParseException& e) {
    LOG_ERROR << "\"" << e.getError() << "\" in [" << filename << ":" << e.getLine() << "]";
    return false;
  } catch (const libconfig::SettingNotFoundException& e) {
    LOG_ERROR << "Setting \"" << e.getPath() << "\" not found in file \"" << filename << "\"";
    return false;
  } catch (...) {
    LOG_ERROR << "Unexpected error in processing config file \"" << filename << "\"";
    return false;
  }

  return true;
}
