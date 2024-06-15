// file   : src/Timer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TIMER_HPP
#define THEGAME_TIMER_HPP

#include "TimePoint.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>

namespace asio = boost::asio;

class Timer final : boost::noncopyable {
public:
  using Handler = std::function<void()>;

  explicit Timer(const asio::any_io_executor& executor);
  Timer(const asio::any_io_executor& executor, Handler handler);
  Timer(const asio::any_io_executor& executor, Handler handler, Duration interval);

  void setInterval(const Duration& interval);
  void setHandler(Handler&& handler);

  void start();
  void stop();

private:
  void tick();

  asio::steady_timer          m_timer;
  Handler                     m_handler;
  Duration                    m_interval {1s};
  TimePoint                   m_expirationTime;
};

#endif /* THEGAME_TIMER_HPP */
