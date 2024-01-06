// file   : TSRoom.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "TSRoom.hpp"

TSRoom::TSRoom(uint32_t id)
  : m_impl(id)
{
}

uint32_t TSRoom::getId() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_impl.getId();
}

void TSRoom::init(const RoomConfig& config)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.init(config);
}

bool TSRoom::hasFreeSpace() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_impl.hasFreeSpace();
}

void TSRoom::join(const SessionPtr& sess)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.join(sess);
}

void TSRoom::leave(const SessionPtr& sess)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.leave(sess);
}

void TSRoom::play(const SessionPtr& sess, const std::string& name, uint8_t color)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.play(sess, name, color);
}

void TSRoom::spectate(const SessionPtr& sess, uint32_t targetId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.spectate(sess, targetId);
}

void TSRoom::pointer(const SessionPtr& sess, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.pointer(sess, point);
}

void TSRoom::eject(const SessionPtr& sess, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.eject(sess, point);
}

void TSRoom::split(const SessionPtr& sess, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.split(sess, point);
}

void TSRoom::chatMessage(const SessionPtr& sess, const std::string& text)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.chatMessage(sess, text);
}

void TSRoom::watch(const SessionPtr& sess, uint32_t playerId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.watch(sess, playerId);
}

void TSRoom::update()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.update();
}
