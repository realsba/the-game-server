// file   : src/RoomManager.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ROOM_MANAGER_HPP
#define THEGAME_ROOM_MANAGER_HPP

#include "Room.hpp"
#include "Config.hpp"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include <memory>
#include <thread>
#include <vector>
#include <mutex>

class RoomManager {
public:
  void start(const RoomConfig& config);
  void stop();

  Room* obtain();
  size_t size() const;

private:
  using Items = std::vector<std::unique_ptr<Room>>;
  using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

  mutable std::mutex          m_mutex;
  asio::io_context            m_ioContext;
  WorkGuard                   m_workGuard {m_ioContext.get_executor()};
  std::vector<std::thread>    m_threads;
  RoomConfig                  m_config;
  Items                       m_items;
  uint32_t                    m_nextId {1};
};

#endif /* THEGAME_ROOM_MANAGER_HPP */
