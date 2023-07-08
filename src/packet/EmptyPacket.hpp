// file   : packet/EmptyPacket.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_EMPTY_PACKET_HPP
#define PACKET_EMPTY_PACKET_HPP

#include "Packet.hpp"

class EmptyPacket : public Packet {
public:
  explicit EmptyPacket(uint8_t type);

  void format(MemoryStream& ms);

  uint8_t type;
};

#endif /* PACKET_EMPTY_PACKET_HPP */
