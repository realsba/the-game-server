// file   : packet/InputPacketTypes.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_INPUT_PACKET_TYPES_HPP
#define THEGAME_PACKET_INPUT_PACKET_TYPES_HPP

struct InputPacketTypes {
  enum {
    Ping = 1,
    ChatMessage = 2,
    Greeting = 3,
    Play = 4,
    Pointer = 5,
    Eject = 6,
    Split = 7,
    Spectate = 8,
    Paint = 9,
    Watch = 10,
  };
};

#endif /* THEGAME_PACKET_INPUT_PACKET_TYPES_HPP */
