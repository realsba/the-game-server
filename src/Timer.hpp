// file   : Timer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef TIMER_HPP
#define TIMER_HPP

#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>

#include <functional>
#include <chrono>
#include <mutex>

class Timer final : private boost::noncopyable {
public:
  typedef std::function<void ()> Handler;

  Timer(boost::asio::io_service& ios);
  Timer(boost::asio::io_service& ios, const Handler& handler);
  Timer(boost::asio::io_service& ios, const Handler& handler, const std::chrono::steady_clock::duration& interval);
  ~Timer();

  void setInterval(const std::chrono::steady_clock::duration& interval);
  void setHandler(const Handler& handler);
  void start();
  void stop();

protected:
  void tick();

protected:
  mutable std::mutex                    m_mutex;
  boost::asio::steady_timer             m_timer;
  std::chrono::steady_clock::time_point m_expiresTime;
  std::chrono::steady_clock::duration   m_interval;
  Handler                               m_handler;
  bool                                  m_started {false};
};

#endif /* TIMER_HPP */
