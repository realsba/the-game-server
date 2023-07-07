// file   : TSRoom.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "TSRoom.hpp"

TSRoom::TSRoom(uint32_t id, WebsocketServer& wss) :
  m_impl(id, wss)
{ }

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

void TSRoom::join(const ConnectionHdl& hdl)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.join(hdl);
}

void TSRoom::leave(const ConnectionHdl& hdl)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.leave(hdl);
}

void TSRoom::play(const ConnectionHdl& hdl, const std::string& name, uint8_t color)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.play(hdl, name, color);
}

void TSRoom::spectate(const ConnectionHdl& hdl, uint32_t targetId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.spectate(hdl, targetId);
}

void TSRoom::pointer(const ConnectionHdl& hdl, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.pointer(hdl, point);
}

void TSRoom::eject(const ConnectionHdl& hdl, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.eject(hdl, point);
}

void TSRoom::split(const ConnectionHdl& hdl, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.split(hdl, point);
}

void TSRoom::chatMessage(const ConnectionHdl& hdl, const std::string& text)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.chatMessage(hdl, text);
}

void TSRoom::watch(const ConnectionHdl& hdl, uint32_t playerId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.watch(hdl, playerId);
}

void TSRoom::paint(const ConnectionHdl& hdl, const Vec2D& point)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.paint(hdl, point);
}

void TSRoom::update()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_impl.update();
}
