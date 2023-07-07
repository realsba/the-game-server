// file   : packet/PacketSpectate.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_PACKET_SPECTATE_HPP
#define PACKET_PACKET_SPECTATE_HPP

#include "Packet.hpp"

#include "src/geometry/Vec2D.hpp"

class Player;

class PacketSpectate : public Packet {
public:
  PacketSpectate(const Player& player);

  void format(MemoryStream& ms);

private:
  const Player& m_player;
};

#endif /* PACKET_PACKET_SPECTATE_HPP */
