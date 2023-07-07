// file   : RoomManager.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef ROOM_MANAGER_HPP
#define ROOM_MANAGER_HPP

#include "RoomConfig.hpp"

#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <vector>

class TSRoom;
class WebsocketServer;

class RoomManager {
public:
  RoomManager(boost::asio::io_service& ios, WebsocketServer& wss);
  ~RoomManager();

  void start(uint32_t numThreads, const RoomConfig& config);
  void stop();

  TSRoom* obtain();
  size_t size() const;

private:
  typedef std::vector<TSRoom*> Items;
  struct Group {
    std::vector<TSRoom*> rooms;
    std::thread thread;
    bool stopped {false};
  };

  mutable std::mutex m_mutex;
  std::vector<Group> m_groups;
  RoomConfig m_config;
  Items m_items;
  boost::asio::io_service& m_ioService;
  WebsocketServer&  m_websocketServer;
  uint32_t m_numThreads {0};
  uint32_t m_nextId {1};
};

#endif /* ROOM_MANAGER_HPP */
