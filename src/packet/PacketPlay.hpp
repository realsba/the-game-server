// file   : packet/PacketPlay.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_PACKET_PLAY_HPP
#define PACKET_PACKET_PLAY_HPP

#include "Packet.hpp"

#include "src/geometry/Vec2D.hpp"

class Player;

class PacketPlay : public Packet {
public:
  explicit PacketPlay(const Player& player);

  void format(MemoryStream& ms);

private:
  const Player& m_player;
};

#endif /* PACKET_PACKET_PLAY_HPP */
