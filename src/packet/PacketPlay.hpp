// file   : packet/PacketPlay.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_PACKET_PLAY_HPP
#define THEGAME_PACKET_PACKET_PLAY_HPP

#include "Packet.hpp"

class Player;

class PacketPlay {
public:
  explicit PacketPlay(const Player& player);

  void format(std::vector<char>& buffer);

private:
  const Player& m_player;
};

#endif /* THEGAME_PACKET_PACKET_PLAY_HPP */
