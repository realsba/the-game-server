// file      : Config.hpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_CONFIG_HPP
#define THEGAME_CONFIG_HPP

#include "MySQLConfig.hpp"
#include "RoomConfig.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <chrono>

class Config {
public:
  Config();

  bool loadFromFile(const std::string& filename);

  boost::asio::ip::tcp::endpoint address;
  std::chrono::milliseconds updateInterval {std::chrono::seconds(1)};
  std::chrono::seconds statisticInterval {std::chrono::minutes(1)};
  std::chrono::seconds connectionTTL {std::chrono::minutes(1)};
  uint listenBacklog {1024};
  uint ioContextThreads {1};
  uint roomThreads {2};

  std::string influxdbServer;
  uint influxdbPort {0};

  MySQLConfig mysql;
  RoomConfig room;
};

#endif /* THEGAME_CONFIG_HPP */
