// file   : packet/EmptyPacket.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_EMPTY_PACKET_HPP
#define THEGAME_PACKET_EMPTY_PACKET_HPP

#include "Packet.hpp"

class EmptyPacket {
public:
  explicit EmptyPacket(uint8_t type);

  void format(std::vector<char>& buffer);

  uint8_t type;
};

#endif /* THEGAME_PACKET_EMPTY_PACKET_HPP */
