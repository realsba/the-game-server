// file   : packet/PacketGreeting.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_PACKET_GREETING_HPP
#define THEGAME_PACKET_PACKET_GREETING_HPP

#include "src/types.hpp"

class PacketGreeting {
public:
  PacketGreeting() = default;
  explicit PacketGreeting(std::string sid);

  void format(Buffer& buffer);

  std::string sid;
};

#endif /* THEGAME_PACKET_PACKET_GREETING_HPP */
