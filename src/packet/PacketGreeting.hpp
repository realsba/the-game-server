// file   : packet/PacketGreeting.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_PACKET_GREETING_HPP
#define PACKET_PACKET_GREETING_HPP

#include <string>

#include "Packet.hpp"

class PacketGreeting : public Packet {
public:
  PacketGreeting() = default;
  PacketGreeting(const std::string& sid);

  void format(MemoryStream& ms);

  std::string sid;
};

#endif /* PACKET_PACKET_GREETING_HPP */
