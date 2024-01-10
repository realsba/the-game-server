// file   : RoomManager.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_MANAGER_HPP
#define THEGAME_ROOM_MANAGER_HPP

#include "TSRoom.hpp"
#include "Config.hpp"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <thread>
#include <vector>
#include <mutex>

class RoomManager {
public:
  void start(uint32_t numThreads, const RoomConfig& config); // TODO: use only RoomConfig
  void stop();

  TSRoom* obtain();
  size_t size() const;

private:
  using Items = std::vector<std::unique_ptr<TSRoom>>;

  mutable std::mutex                    m_mutex;
  asio::io_context                      m_ioContext;
  std::vector<std::thread>              m_threads;
  RoomConfig                            m_config;
  Items                                 m_items;
  uint32_t                              m_nextId {1};
};

#endif /* THEGAME_ROOM_MANAGER_HPP */
