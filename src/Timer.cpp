// file   : Timer.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Timer.hpp"

#include <boost/asio/post.hpp>

Timer::Timer(const asio::any_io_executor& executor)
  : m_timer(executor)
{
}

Timer::Timer(const asio::any_io_executor& executor, Handler handler)
  : m_timer(executor)
  , m_handler(std::move(handler))
{
}

Timer::Timer(const asio::any_io_executor& executor, Handler handler, Duration interval)
  : m_timer(executor)
  , m_interval(interval)
  , m_handler(std::move(handler))
{
}

void Timer::setInterval(const Duration& interval)
{
  asio::post(m_timer.get_executor(),
    [this, interval]
    {
      m_interval = interval;
    }
  );
}

void Timer::setHandler(Handler&& handler)
{
  asio::post(m_timer.get_executor(),
    [this, handler = std::move(handler)]() mutable
    {
      m_handler = std::move(handler);
    }
  );
}

void Timer::start()
{
  asio::post(m_timer.get_executor(),
    [&]
    {
      m_expirationTime = TimePoint::clock::now();
      tick();
    }
  );
}

void Timer::stop()
{
  asio::post(m_timer.get_executor(), [&] { m_timer.cancel(); });
}

void Timer::tick()
{
  m_expirationTime += m_interval;
  m_timer.expires_at(m_expirationTime);
  m_timer.async_wait(
    [&](const boost::system::error_code& ec)
    {
      if (!ec) {
        m_handler();
        tick();
      }
    }
  );
}
