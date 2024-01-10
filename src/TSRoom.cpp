// file   : TSRoom.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "TSRoom.hpp"

#include <boost/asio/bind_executor.hpp>

using namespace std::chrono_literals;

TSRoom::TSRoom(asio::io_context& ioc, uint32_t id)
  : m_strand(ioc)
  , m_impl(id)
  , m_timer(ioc)
{
}

uint32_t TSRoom::getId() const
{
  return m_impl.getId();
}

void TSRoom::init(const RoomConfig& config)
{
  m_config = config;
  m_impl.init(config);
}

bool TSRoom::hasFreeSpace() const
{
  return m_impl.hasFreeSpace();
}

void TSRoom::join(const SessionPtr& sess)
{
  m_strand.post(std::bind_front(&Room::join, &m_impl, sess));
}

void TSRoom::leave(const SessionPtr& sess)
{
  m_strand.post(std::bind_front(&Room::leave, &m_impl, sess));
}

void TSRoom::play(const SessionPtr& sess, const std::string& name, uint8_t color)
{
  m_strand.post(std::bind_front(&Room::play, &m_impl, sess, name, color));
}

void TSRoom::spectate(const SessionPtr& sess, uint32_t targetId)
{
  m_strand.post(std::bind_front(&Room::spectate, &m_impl, sess, targetId));
}

void TSRoom::pointer(const SessionPtr& sess, const Vec2D& point)
{
  m_strand.post(std::bind_front(&Room::pointer, &m_impl, sess, point));
}

void TSRoom::eject(const SessionPtr& sess, const Vec2D& point)
{
  m_strand.post([this, sess, point] { m_impl.eject(sess, point); });
}

void TSRoom::split(const SessionPtr& sess, const Vec2D& point)
{
  m_strand.post([this, sess, point] { m_impl.split(sess, point); });
}

void TSRoom::chatMessage(const SessionPtr& sess, const std::string& text)
{
  m_strand.post(std::bind_front(&Room::chatMessage, &m_impl, sess, text));
}

void TSRoom::watch(const SessionPtr& sess, uint32_t playerId)
{
  m_strand.post(std::bind_front(&Room::watch, &m_impl, sess, playerId));
}

void TSRoom::start()
{
  update();
}

void TSRoom::stop()
{
  m_strand.post([this] { m_timer.cancel(); });
}

void TSRoom::update()
{
  m_expirationTime += m_config.tickInterval;
  m_timer.expires_at(m_expirationTime);
  m_timer.async_wait(asio::bind_executor(m_strand, [&](const boost::system::error_code& error) {
    if (!error) {
      m_impl.update();
      update();
    }
  }));
}
