// file   : TSRoom.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef TS_ROOM_HPP
#define TS_ROOM_HPP

#include "Room.hpp"

#include <mutex>

class TSRoom {
public:
  TSRoom(uint32_t id, WebsocketServer& wss);

  uint32_t getId() const;
  void init(const RoomConfig& config);
  bool hasFreeSpace() const;

  void join(const ConnectionHdl& hdl);
  void leave(const ConnectionHdl& hdl);
  void play(const ConnectionHdl& hdl, const std::string& name, uint8_t color);
  void spectate(const ConnectionHdl& hdl, uint32_t targetId);
  void pointer(const ConnectionHdl& hdl, const Vec2D& point);
  void eject(const ConnectionHdl& hdl, const Vec2D& point);
  void split(const ConnectionHdl& hdl, const Vec2D& point);
  void chatMessage(const ConnectionHdl& hdl, const std::string& text);
  void watch(const ConnectionHdl& hdl, uint32_t playerId);
  void paint(const ConnectionHdl& hdl, const Vec2D& point);

  void update();

private:
  mutable std::mutex m_mutex;
  Room m_impl;
};

#endif /* TS_ROOM_HPP */
