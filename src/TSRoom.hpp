// file   : TSRoom.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TS_ROOM_HPP
#define THEGAME_TS_ROOM_HPP

#include "Room.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

namespace asio = boost::asio;

class TSRoom {
public:
  TSRoom(asio::io_context& ioc, uint32_t id);

  uint32_t getId() const;
  void init(const RoomConfig& config);
  bool hasFreeSpace() const;

  void join(const SessionPtr& sess);
  void leave(const SessionPtr& sess);
  void play(const SessionPtr& sess, const std::string& name, uint8_t color);
  void spectate(const SessionPtr& sess, uint32_t targetId);
  void point(const SessionPtr& sess, const Vec2D& point);
  void eject(const SessionPtr& sess, const Vec2D& point);
  void split(const SessionPtr& sess, const Vec2D& point);
  void chatMessage(const SessionPtr& sess, const std::string& text);
  void watch(const SessionPtr& sess, uint32_t playerId);

  void start();
  void stop();

private:
  void update();

private:
  asio::io_context::strand    m_strand;
  asio::steady_timer          m_timer;
  RoomConfig                  m_config;
  Room                        m_impl;
  TimePoint                   m_expirationTime {TimePoint::clock::now()};
};

#endif /* THEGAME_TS_ROOM_HPP */
