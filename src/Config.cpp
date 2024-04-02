// file      : src/Config.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#include "Config.hpp"

#include <toml.hpp>

using namespace boost;
using namespace std::chrono;

namespace toml {

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
struct from<config::Server>
{
  static auto from_toml(const value& v)
  {
    config::Server result{};

    const auto host = find<std::string>(v, "host");
    const auto port = find<uint16_t>(v, "port");
    result.address = asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port);
    result.numThreads = find<uint32_t>(v, "numThreads");

    return result;
  }
};

template <>
struct from<config::InfluxDb>
{
  static auto from_toml(value& v)
  {
    config::InfluxDb result{};

    result.enabled = find_or<bool>(v, "enabled", true);
    result.host = find<std::string>(v, "host");
    result.port = find<uint16_t>(v, "port");
    result.path = find<std::string>(v, "path");
    result.token = find<std::string>(v, "token");
    result.interval = find<Duration>(v, "interval");

    return result;
  }
};

template <>
struct from<config::MySql>
{
  static auto from_toml(const value& v)
  {
    config::MySql result{};

    result.database     = find<std::string>(v, "database");
    result.host         = find<std::string>(v, "host");
    result.port         = find<uint>(v, "port");
    result.user         = find<std::string>(v, "user");
    result.password     = find<std::string>(v, "password");
    result.charset      = find<std::string>(v, "charset");
    result.maxIdleTime  = find<uint>(v, "maxIdleTime");

    return result;
  }
};

template <>
struct from<config::Leaderboard>
{
  static auto from_toml(value& v)
  {
    config::Leaderboard result{};

    result.limit = find<uint32_t>(v, "limit");
    result.updateInterval = find<Duration>(v, "updateInterval");

    return result;
  }
};

template <>
struct from<config::Player>
{
  static auto from_toml(value& v)
  {
    config::Player result{};

    result.mass                   = find<uint32_t>(v, "mass");
    result.maxAvatars             = find<uint32_t>(v, "maxAvatars");
    result.deflationThreshold     = find<Duration>(v, "deflationThreshold");
    result.deflationInterval      = find<Duration>(v, "deflationInterval");
    result.deflationRatio         = find<float>(v, "deflationRatio");
    result.annihilationThreshold  = find<Duration>(v, "annihilationThreshold");
    result.pointerForceRatio      = find<float>(v, "pointerForceRatio");

    return result;
  }
};

template <>
struct from<config::Bot>
{
  static auto from_toml(value& v)
  {
    config::Bot result{};

    result.mass = find<uint32_t>(v, "mass");
    result.respawnDelay = find<Duration>(v, "respawnDelay");

    return result;
  }
};

template <>
struct from<config::Avatar>
{
  static auto from_toml(value& v)
  {
    config::Avatar result{};

    result.recombinationDuration = find<Duration>(v, "recombinationDuration");
    result.minVelocity        = find<uint32_t>(v, "minVelocity");
    result.maxVelocity        = find<uint32_t>(v, "maxVelocity");
    result.explosionMinMass   = find<uint32_t>(v, "explosionMinMass");
    result.explosionParts     = find<uint32_t>(v, "explosionParts");
    result.splitMinMass       = find<uint32_t>(v, "splitMinMass");
    result.splitVelocity      = find<uint32_t>(v, "splitVelocity");
    result.ejectionMinMass    = find<uint32_t>(v, "ejectionMinMass");
    result.ejectionVelocity   = find<uint32_t>(v, "ejectionVelocity");
    result.ejectionMass       = find<uint32_t>(v, "ejectionMass");
    result.ejectionMassLoss   = find<uint32_t>(v, "ejectionMassLoss");

    return result;
  }
};

template <>
struct from<config::Food>
{
  static auto from_toml(value& v)
  {
    config::Food result{};

    result.mass             = find<uint32_t>(v, "mass");
    result.radius           = find<uint32_t>(v, "radius");
    result.quantity         = find<uint32_t>(v, "quantity");
    result.maxQuantity      = find<uint32_t>(v, "maxQuantity");
    result.minVelocity      = find<uint32_t>(v, "minVelocity");
    result.maxVelocity      = find<uint32_t>(v, "maxVelocity");
    result.resistanceRatio  = find<float>(v, "resistanceRatio");
    result.minColorIndex    = find<int>(v, "minColorIndex");
    result.maxColorIndex    = find<int>(v, "maxColorIndex");

    return result;
  }
};

template <>
struct from<config::Virus>
{
  static auto from_toml(value& v)
  {
    config::Virus result{};

    result.mass         = find<uint32_t>(v, "mass");
    result.quantity     = find<uint32_t>(v, "quantity");
    result.maxQuantity  = find<uint32_t>(v, "maxQuantity");
    result.lifeTime     = find<Duration>(v, "lifeTime");
    result.color        = find<uint32_t>(v, "color");

    return result;
  }
};

template <>
struct from<config::Mother>
{
  static auto from_toml(value& v)
  {
    config::Mother result{};

    result.mass               = find<uint32_t>(v, "mass");
    result.quantity           = find<uint32_t>(v, "quantity");
    result.maxMass            = find<uint32_t>(v, "maxMass");
    result.maxQuantity        = find<uint32_t>(v, "maxQuantity");
    result.lifeTime           = find<Duration>(v, "lifeTime");
    result.color              = find<uint32_t>(v, "color");
    result.checkRadius        = find<uint32_t>(v, "checkRadius");
    result.nearbyFoodLimit    = find<uint32_t>(v, "nearbyFoodLimit");
    result.baseFoodProduction = find<float>(v, "baseFoodProduction");
    result.foodCheckInterval  = find<Duration>(v, "foodCheckInterval");
    result.foodGenerationInterval = find<Duration>(v, "foodGenerationInterval");

    return result;
  }
};

template <>
struct from<config::Generator::Item>
{
  static auto from_toml(value& v)
  {
    config::Generator::Item result{};

    result.interval = find<Duration>(v, "interval");
    result.quantity = find<uint32_t>(v, "quantity");
    result.enabled  = find_or<bool>(v, "enabled", true);

    return result;
  }
};

template <>
struct from<config::Generator>
{
  static auto from_toml(value& v)
  {
    config::Generator result{};

    result.food = find<config::Generator::Item>(v, "food");
    result.virus = find<config::Generator::Item>(v, "virus");
    result.phage = find<config::Generator::Item>(v, "phage");
    result.mother = find<config::Generator::Item>(v, "mother");

    return result;
  }
};

template <>
struct from<config::Room>
{
  static auto from_toml(value& v)
  {
    config::Room result{};

    result.numThreads = find<uint32_t>(v, "numThreads");
    if (result.numThreads < 1) {
      throw std::runtime_error("room.numThreads should be > 0");
    }

    result.updateInterval = find<Duration>(v, "updateInterval");
    if (result.updateInterval == Duration::zero()) {
      throw std::runtime_error("room.updateInterval should be > 0");
    }

    result.syncInterval = find<Duration>(v, "syncInterval");

    result.spawnPosTryCount             = find<uint32_t>(v, "spawnPosTryCount");
    result.checkExpirableCellsInterval  = find<Duration>(v, "checkExpirableCellsInterval");

    result.viewportBase         = find<uint32_t>(v, "viewportBase");
    result.viewportBuffer       = find<float>(v, "viewportBuffer");
    result.aspectRatio          = find<float>(v, "aspectRatio");

    result.width                = find<uint32_t>(v, "width");
    result.height               = find<uint32_t>(v, "height");
    result.maxMass              = find<uint32_t>(v, "maxMass");
    result.maxPlayers           = find<uint32_t>(v, "maxPlayers");
    result.maxRadius            = find<uint32_t>(v, "maxRadius");
    result.scaleRatio           = find<float>(v, "scaleRatio");
    result.explodeVelocity      = find<uint32_t>(v, "explodeVelocity");
    result.resistanceRatio      = find<float>(v, "resistanceRatio");
    result.elasticityRatio      = find<float>(v, "elasticityRatio");
    result.cellMinMass          = find<uint32_t>(v, "cellMinMass");
    result.cellRadiusRatio      = find<float>(v, "cellRadiusRatio");
    result.botNames             = find_or<config::Room::BotNames>(v, "botNames", {});

    result.leaderboard = find<config::Leaderboard>(v, "leaderboard");
    result.player     = find<config::Player>(v, "player");
    result.bot        = find<config::Bot>(v, "bot");
    result.avatar     = find<config::Avatar>(v, "avatar");
    result.food       = find<config::Food>(v, "food");
    result.virus      = find<config::Virus>(v, "virus");
    result.phage      = find<config::Phage>(v, "phage");
    result.mother     = find<config::Mother>(v, "mother");
    result.generator  = find<config::Generator>(v, "generator");

    result.simulationInterval = std::chrono::duration_cast<std::chrono::duration<double>>(result.updateInterval).count();
    result.cellMinRadius = result.cellRadiusRatio * sqrt(result.cellMinMass / M_PI);
    result.cellMaxRadius = result.cellRadiusRatio * sqrt(result.maxMass / M_PI);
    result.cellRadiusDiff = result.cellMaxRadius - result.cellMinRadius;
    result.avatarVelocityDiff = result.avatar.maxVelocity - result.avatar.minVelocity;

    return result;
  }
};

} // namespace toml

namespace config {

void Config::load(const std::string& filename)
{
  auto data = toml::parse(filename);

  server    = toml::find<config::Server>(data, "server");
  mysql     = toml::find<config::MySql>(data, "mysql");
  influxdb  = toml::find<config::InfluxDb>(data, "influxdb");
  room      = toml::find<config::Room>(data, "room");
}

} // namespace config
