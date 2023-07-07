// file   : packet/PacketLeaderboard.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_PACKET_LEADERBOARD_HPP
#define PACKET_PACKET_LEADERBOARD_HPP

#include <vector>

#include "Packet.hpp"

class Player;

class PacketLeaderboard : public Packet {
public:
  void format(MemoryStream& ms, const std::vector<Player*>& items, uint32_t max);
};

#endif /* PACKET_PACKET_LEADERBOARD_HPP */
