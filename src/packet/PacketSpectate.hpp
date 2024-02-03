// file   : src/packet/PacketSpectate.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_PACKET_SPECTATE_HPP
#define THEGAME_PACKET_PACKET_SPECTATE_HPP

#include "src/types.hpp"

#include "src/geometry/Vec2D.hpp"

class Player;

class PacketSpectate {
public:
  explicit PacketSpectate(const Player& player);

  void format(Buffer& buffer);

private:
  const Player& m_player;
};

#endif /* THEGAME_PACKET_PACKET_SPECTATE_HPP */
