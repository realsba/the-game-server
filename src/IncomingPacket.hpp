// file   : src/IncomingPacket.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_INCOMING_PACKET_HPP
#define THEGAME_INCOMING_PACKET_HPP

namespace IncomingPacket {

enum Type {
  Ping = 1,
  ChatMessage = 2,
  Greeting = 3,
  Play = 4,
  Move = 5,
  Eject = 6,
  Split = 7,
  Spectate = 8,
  Watch = 10,
};

} // namespace IncomingPacket

#endif /* THEGAME_INCOMING_PACKET_HPP */
