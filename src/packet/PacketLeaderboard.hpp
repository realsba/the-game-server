// file   : packet/PacketLeaderboard.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_PACKET_LEADERBOARD_HPP
#define THEGAME_PACKET_PACKET_LEADERBOARD_HPP

#include "src/types.hpp"

class Player;

class PacketLeaderboard {
public:
  PacketLeaderboard(const std::vector<Player*>& items, size_t limit);

  void format(Buffer& buffer);

private:
  const std::vector<Player*>& m_items;
  const size_t m_limit {0};
};

#endif /* THEGAME_PACKET_PACKET_LEADERBOARD_HPP */
