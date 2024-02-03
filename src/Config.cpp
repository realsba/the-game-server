// file      : src/Config.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include "Config.hpp"

#include <spdlog/spdlog.h>
#include <toml.hpp>
#include <iostream>

using namespace boost;
using namespace std::chrono;

namespace toml
{
  template <>
  struct from<Duration>
  {
    static auto from_toml(const value& v)
    {
      std::istringstream iss(v.as_string());
      Duration duration {};
      std::string unit;
      long long value;

      while (iss >> value >> unit) {
        if (unit == "h") {
          duration += hours(value);
        } else if (unit == "m") {
          duration += minutes(value);
        } else if (unit == "s") {
          duration += seconds(value);
        } else if (unit == "ms") {
          duration += milliseconds(value);
        } else if (unit == "µs" || unit == "us") {
          duration += microseconds(value);
        }

        if (iss.peek() == ':' || iss.peek() == ' ' || iss.peek() == ',') {
          iss.ignore();
        }
      }

      return duration;
    }
  };

  template <>
  struct from<MySQLConfig>
  {
    static auto from_toml(const value& v)
    {
      MySQLConfig result{};

      result.database     = find<std::string>(v, "database");
      result.server       = find<std::string>(v, "server");
      result.user         = find<std::string>(v, "user");
      result.password     = find<std::string>(v, "password");
      result.charset      = find<std::string>(v, "charset");
      result.port         = find<uint>(v, "port");
      result.maxIdleTime  = find<uint>(v, "maxIdleTime");

      return result;
    }
  };

  template <>
  struct from<RoomConfig>
  {
    static auto from_toml(value& v)
    {
      RoomConfig result{};

      result.numThreads = find<uint32_t>(v, "numThreads");
      if (result.numThreads < 1) {
        throw std::runtime_error("room.numThreads should be > 0");
      }

      result.updateInterval = find<Duration>(v, "updateInterval");
      if (result.updateInterval == Duration::zero()) {
        throw std::runtime_error("room.updateInterval should be > 0");
      }

      result.simulationsPerUpdate         = find<uint32_t>(v, "simulationsPerUpdate");
      result.spawnPosTryCount             = find<uint32_t>(v, "spawnPosTryCount");
      result.checkPlayersInterval         = find<Duration>(v, "checkPlayersInterval");
      result.updateLeaderboardInterval    = find<Duration>(v, "updateLeaderboardInterval");
      result.destroyOutdatedCellsInterval = find<Duration>(v, "destroyOutdatedCellsInterval");
      result.checkMothersInterval         = find<Duration>(v, "checkMothersInterval");
      result.produceMothersInterval       = find<Duration>(v, "produceMothersInterval");

      result.viewportBase                 = find<uint32_t>(v, "viewportBase");
      result.viewportBuffer               = find<float>(v, "viewportBuffer");
      result.aspectRatio                  = find<float>(v, "aspectRatio");

      result.width                        = find<uint32_t>(v, "width");
      result.height                       = find<uint32_t>(v, "height");
      result.maxMass                      = find<uint32_t>(v, "maxMass");
      result.maxPlayers                   = find<uint32_t>(v, "maxPlayers");
      result.maxRadius                    = find<uint32_t>(v, "maxRadius");
      result.leaderboardVisibleItems      = find<uint32_t>(v, "leaderboardVisibleItems");
      result.scaleRatio                   = find<float>(v, "scaleRatio");
      result.explodeVelocity              = find<uint32_t>(v, "explodeVelocity");

      result.playerMaxCells               = find<uint32_t>(v, "playerMaxCells");
      result.playerDeflationInterval      = find<Duration>(v, "playerDeflationInterval");
      result.playerDeflationRatio         = find<float>(v, "playerDeflationRatio");
      result.playerAnnihilationInterval   = find<Duration>(v, "playerAnnihilationInterval");
      result.playerForceRatio             = find<float>(v, "playerForceRatio");

      result.botNames                     = toml::find<RoomConfig::BotNames>(v, "botNames");
      result.botStartMass                 = find<uint32_t>(v, "botStartMass");
      result.botForceCornerRatio          = find<float>(v, "botForceCornerRatio");
      result.botForceFoodRatio            = find<float>(v, "botForceFoodRatio");
      result.botForceHungerRatio          = find<float>(v, "botForceHungerRatio");
      result.botForceDangerRatio          = find<float>(v, "botForceDangerRatio");
      result.botForceStarRatio            = find<float>(v, "botForceStarRatio");

      result.resistanceRatio      = find<float>(v, "resistanceRatio");
      result.elasticityRatio      = find<float>(v, "elasticityRatio");

      result.cellMinMass          = find<uint32_t>(v, "cellMinMass");
      result.cellRadiusRatio      = find<float>(v, "cellRadiusRatio");

      result.avatarStartMass      = find<uint32_t>(v, "avatarStartMass");
      result.avatarMinSpeed       = find<uint32_t>(v, "avatarMinSpeed");
      result.avatarMaxSpeed       = find<uint32_t>(v, "avatarMaxSpeed");
      result.avatarExplodeMinMass = find<uint32_t>(v, "avatarExplodeMinMass");
      result.avatarExplodeParts   = find<uint32_t>(v, "avatarExplodeParts");
      result.avatarSplitMinMass   = find<uint32_t>(v, "avatarSplitMinMass");
      result.avatarSplitVelocity  = find<uint32_t>(v, "avatarSplitVelocity");
      result.avatarEjectMinMass   = find<uint32_t>(v, "avatarEjectMinMass");
      result.avatarEjectVelocity  = find<uint32_t>(v, "avatarEjectVelocity");
      result.avatarEjectMass      = find<uint32_t>(v, "avatarEjectMass");
      result.avatarEjectMassLoss  = find<uint32_t>(v, "avatarEjectMassLoss");
      result.avatarRecombineTime  = find<uint32_t>(v, "avatarRecombineTime");

      result.foodStartAmount      = find<uint32_t>(v, "foodStartAmount");
      result.foodMaxAmount        = find<uint32_t>(v, "foodMaxAmount");
      result.foodMass             = find<uint32_t>(v, "foodMass");
      result.foodRadius           = find<uint32_t>(v, "foodRadius");
      result.foodMinVelocity      = find<uint32_t>(v, "foodMinVelocity");
      result.foodMaxVelocity      = find<uint32_t>(v, "foodMaxVelocity");
      result.foodResistanceRatio  = find<float>(v, "foodResistanceRatio");
      if (result.foodMinVelocity > result.foodMaxVelocity) {
        spdlog::warn("room.foodMinVelocity > room.foodMaxVelocity");
      }

      result.virusStartMass       = find<uint32_t>(v, "virusStartMass");
      result.virusStartAmount     = find<uint32_t>(v, "virusStartAmount");
      result.virusMaxAmount       = find<uint32_t>(v, "virusMaxAmount");
      result.virusLifeTime        = find<Duration>(v, "virusLifeTime");
      result.virusColor           = find<uint32_t>(v, "virusColor");

      result.phageStartMass       = find<uint32_t>(v, "phageStartMass");
      result.phageStartAmount     = find<uint32_t>(v, "phageStartAmount");
      result.phageMaxAmount       = find<uint32_t>(v, "phageMaxAmount");
      result.phageLifeTime        = find<Duration>(v, "phageLifeTime");
      result.phageColor           = find<uint32_t>(v, "phageColor");

      result.motherStartMass      = find<uint32_t>(v, "motherStartMass");
      result.motherStartAmount    = find<uint32_t>(v, "motherStartAmount");
      result.motherMaxAmount      = find<uint32_t>(v, "motherMaxAmount");
      result.motherLifeTime       = find<Duration>(v, "motherLifeTime");
      result.motherExplodeMass    = find<uint32_t>(v, "motherExplodeMass");
      result.motherColor          = find<uint32_t>(v, "motherColor");
      result.motherCheckRadius    = find<uint32_t>(v, "motherCheckRadius");

      result.spawnFoodMass        = find<float>(v, "spawnFoodMass");
      result.spawnVirusMass       = find<float>(v, "spawnVirusMass");
      result.spawnPhageMass       = find<float>(v, "spawnPhageMass");
      result.spawnMotherMass      = find<float>(v, "spawnMotherMass");

      return result;
    }
  };
} // namespace toml

void Config::load(const std::string& filename)
{
  auto data = toml::parse("thegame.toml");

  const auto host = toml::find<std::string>(data, "host");
  const auto port = toml::find<uint16_t>(data, "port");
  address = asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port);

  numThreads = toml::find<uint32_t >(data, "numThreads");
  if (numThreads < 1) {
    throw std::runtime_error("numThreads should be > 0");
  }

  statisticInterval = toml::find<Duration>(data, "statisticInterval");
  if (statisticInterval == Duration::zero()) {
    throw std::runtime_error("statisticInterval should be > 0");
  }

  influxdbServer = toml::find<std::string>(data, "influxdb", "server");
  influxdbPort = toml::find<uint16_t>(data, "influxdb", "port");

  mysql = toml::find<MySQLConfig>(data, "mysql");
  room = toml::find<RoomConfig>(data, "room");
}
