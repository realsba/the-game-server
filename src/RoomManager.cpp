// file   : RoomManager.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "RoomManager.hpp"

#include <spdlog/spdlog.h>

void RoomManager::start(uint32_t numThreads, const RoomConfig& config)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // * TODO: remove
  auto* room = new TSRoom(m_ioContext, m_nextId++);
  room->init(config);
  room->start();
  m_items.emplace_back(room);
  // * TODO: remove

  m_config = config;
  m_threads.reserve(numThreads);
  for (uint i = 0; i < numThreads; ++i) {
    m_threads.emplace_back(
      [this]
      {
        spdlog::info("Start \"Room worker\"");
        while (true) {
          try {
            m_ioContext.run();
            break;
          } catch (const std::exception& e) {
            spdlog::error("Exception caught in RoomManager: {}", e.what());
          }
        }
        spdlog::info("Stop \"Room worker\"");
      }
    );
  }
}

void RoomManager::stop()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (const auto& room : m_items) {
    room->stop();
  }
  m_ioContext.stop();
  for (auto& thread : m_threads) {
    thread.join();
  }
  m_threads.clear();
  m_ioContext.reset();
}

TSRoom* RoomManager::obtain()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (const auto& room : m_items) {
    if (room->hasFreeSpace()) {
      return room.get();
    }
  }
  auto room = std::make_unique<TSRoom>(m_ioContext, m_nextId++);
  room->init(m_config);
  room->start();
  m_items.emplace_back(std::move(room));
  return m_items.back().get();
}

size_t RoomManager::size() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_items.size();
}