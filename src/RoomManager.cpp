// file   : RoomManager.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "RoomManager.hpp"
#include "TSRoom.hpp"
#include "Logger.hpp"

#include <sys/syscall.h>

RoomManager::RoomManager(boost::asio::io_service& ios, WebsocketServer& wss) :
  m_ioService(ios),
  m_websocketServer(wss)
{ }

RoomManager::~RoomManager()
{
  for (TSRoom* room: m_items) {
    delete room;
  }
}

void RoomManager::start(uint32_t numThreads, const RoomConfig& config)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_numThreads = numThreads;
  m_config = config;
  m_groups.resize(numThreads);
  for (uint i = 0; i < numThreads; ++i) {
    m_groups[i].thread = std::thread(
      [&stopped=m_groups[i].stopped, &rooms=m_groups[i].rooms]()
      {
        long int pid = syscall(SYS_gettid);
        LOG_INFO << "Start \"Room worker\" (" << pid << ")";
        while (!stopped) {
          try {
            TimePoint start = TimePoint::clock::now();
            for (TSRoom* room : rooms) {
              room->update();
            }
            auto dt = TimePoint::clock::now() - start;
            if (dt < std::chrono::milliseconds(5)) {
              std::this_thread::sleep_for(std::chrono::milliseconds(5) - dt);
            }
          } catch (const std::exception& e) {
            LOG_ERROR << e.what();
          }
        }
        LOG_INFO << "Stop \"Room worker\" (" << pid << ")";
      }
    );
  }
}

void RoomManager::stop()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (Group& group : m_groups) {
    group.stopped = true;
    group.thread.join();
  }
  m_groups.clear();
}

TSRoom* RoomManager::obtain()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (TSRoom* room: m_items) {
    if (room->hasFreeSpace()) {
      return room;
    }
  }
  TSRoom* room = new TSRoom(m_nextId++, m_websocketServer);
  room->init(m_config);
  m_items.emplace_back(room);
  m_groups.at(room->getId() % m_numThreads).rooms.push_back(room);
  return room;
}

size_t RoomManager::size() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_items.size();
}