// file   : TSRoom.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TS_ROOM_HPP
#define THEGAME_TS_ROOM_HPP

#include "SessionFwd.hpp"
#include "Room.hpp"

#include <mutex>

class TSRoom {
public:
  explicit TSRoom(uint32_t id);

  uint32_t getId() const;
  void init(const RoomConfig& config);
  bool hasFreeSpace() const;

  void join(const SessionPtr& sess);
  void leave(const SessionPtr& sess);
  void play(const SessionPtr& sess, const std::string& name, uint8_t color);
  void spectate(const SessionPtr& sess, uint32_t targetId);
  void pointer(const SessionPtr& sess, const Vec2D& point);
  void eject(const SessionPtr& sess, const Vec2D& point);
  void split(const SessionPtr& sess, const Vec2D& point);
  void chatMessage(const SessionPtr& sess, const std::string& text);
  void watch(const SessionPtr& sess, uint32_t playerId);

  void update();

private:
  mutable std::mutex m_mutex;
  Room m_impl;
};

#endif /* THEGAME_TS_ROOM_HPP */
