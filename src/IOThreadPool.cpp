// file   : src/IOThreadPool.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "IOThreadPool.hpp"

#include <spdlog/spdlog.h>

IOThreadPool::IOThreadPool(std::string name, asio::io_context& ioc)
  : m_name(std::move(name))
  , m_ioContext(ioc)
{}

void IOThreadPool::start(uint32_t numThreads)
{
  m_threads.reserve(numThreads);
  for (uint i = 0; i < numThreads; ++i) {
    m_threads.emplace_back(
      [this]
      {
        spdlog::info("Start \"{}\"", m_name);
        while (true) {
          try {
            m_ioContext.run();
            break;
          } catch (const std::exception& e) {
            spdlog::error("Error in \"{}\": {}", m_name, e.what());
          }
        }
        spdlog::info("Stop \"{}\"", m_name);
      }
    );
  }
}

void IOThreadPool::stop()
{
  m_ioContext.stop();
  for (auto& thread : m_threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  m_threads.clear();
  m_ioContext.reset();
}
