// file   : src/IOThreadPool.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_IO_THREAD_POOL_HPP
#define THEGAME_IO_THREAD_POOL_HPP

#include <boost/asio/io_context.hpp>

#include <thread>
#include <vector>

namespace asio = boost::asio;

class IOThreadPool {
public:
  IOThreadPool(std::string name, asio::io_context& ioc);

  void start(uint32_t numThreads);
  void stop();

private:
  const std::string           m_name;
  std::vector<std::thread>    m_threads;
  asio::io_context&           m_ioContext;
};

#endif /* THEGAME_IO_THREAD_POOL_HPP */
