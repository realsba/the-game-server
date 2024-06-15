// file   : src/RoomManager.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "RoomManager.hpp"

#include <boost/asio/strand.hpp>

#include <spdlog/spdlog.h>

void RoomManager::start(const config::Room& config)
{
  std::lock_guard lock(m_mutex);
  m_config = config;
  m_ioThreadPool.start(config.numThreads);
}

void RoomManager::stop()
{
  std::lock_guard lock(m_mutex);
  for (const auto& room : m_items) {
    room->stop();
  }
  m_ioThreadPool.stop();
}

Room* RoomManager::obtain()
{
  std::lock_guard lock(m_mutex);

  for (const auto& room : m_items) {
    if (room->hasFreeSpace()) {
      return room.get();
    }
  }

  auto room = std::make_unique<Room>(asio::make_strand(m_ioContext), m_nextId++);
  room->init(m_config);
  room->start();

  if (m_items.empty()) {
    m_workGuard.reset();
  }

  m_items.emplace_back(std::move(room));

  return m_items.back().get();
}

size_t RoomManager::size() const
{
  std::lock_guard lock(m_mutex);
  return m_items.size();
}