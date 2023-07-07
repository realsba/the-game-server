// file   : Timer.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Timer.hpp"

Timer::Timer(boost::asio::io_service& ios) :
  m_timer(ios)
{ }

Timer::Timer(boost::asio::io_service& ios, const Handler& handler) :
  m_timer(ios),
  m_handler(handler)
{ }

Timer::Timer(boost::asio::io_service& ios, const Handler& handler, const std::chrono::steady_clock::duration& interval) :
  m_timer(ios),
  m_interval(interval),
  m_handler(handler)
{ }

Timer::~Timer()
{
  stop();
}

void Timer::setInterval(const std::chrono::steady_clock::duration& interval)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_interval = interval;
}

void Timer::setHandler(const Handler& handler)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_handler = handler;
}

void Timer::start()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_started) {
    m_expiresTime = std::chrono::steady_clock::now();
    m_started = true;
    tick();
  }
}

void Timer::stop()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_started) {
    m_started = false;
    m_timer.cancel();
  }
}

void Timer::tick()
{
  if (!m_started) {
    return;
  }
  m_expiresTime += m_interval;
  m_timer.expires_at(m_expiresTime);
  m_timer.async_wait(
    [this, handler = m_handler] (const boost::system::error_code & ec)
    {
      if (!ec) {
        if (handler) {
          handler();
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        tick();
      }
    }
  );
}
