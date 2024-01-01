// file   : Timer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TIMER_HPP
#define THEGAME_TIMER_HPP

#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>

#include <functional>
#include <chrono>
#include <mutex>

class Timer final : private boost::noncopyable {
public:
  using Handler = std::function<void()>;

  explicit Timer(boost::asio::io_context& ioc);
  Timer(boost::asio::io_context& ioc, Handler handler);
  Timer(boost::asio::io_context& ioc, Handler handler, const std::chrono::steady_clock::duration& interval);
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
  std::chrono::steady_clock::duration   m_interval {std::chrono::seconds(30)};
  Handler                               m_handler;
  bool                                  m_started {false};
};

#endif /* THEGAME_TIMER_HPP */
