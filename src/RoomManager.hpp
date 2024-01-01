// file   : RoomManager.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_MANAGER_HPP
#define THEGAME_ROOM_MANAGER_HPP

#include "RoomConfig.hpp"

#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <vector>

class TSRoom;

class RoomManager {
public:
  ~RoomManager();

  void start(uint32_t numThreads, const RoomConfig& config);
  void stop();

  TSRoom* obtain();
  size_t size() const;

private:
  using Items = std::vector<TSRoom*>;

  struct Group {
    std::vector<TSRoom*> rooms;
    std::thread thread;
    bool stopped {false};
  };

  mutable std::mutex                    m_mutex;
  std::vector<Group>                    m_groups;
  RoomConfig                            m_config;
  Items                                 m_items;
  uint32_t                              m_numThreads {0};
  uint32_t                              m_nextId {1};
};

#endif /* THEGAME_ROOM_MANAGER_HPP */
