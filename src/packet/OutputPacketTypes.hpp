// file   : packet/OutputPacketTypes.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_OUTPUT_PACKET_TYPES_HPP
#define PACKET_OUTPUT_PACKET_TYPES_HPP

struct OutputPacketTypes {
  enum {
    Pong = 1,
    Greeting = 2,
    Room = 3,
    Frame = 4,
    Leaderboard = 5,
    Player = 6,
    PlayerRemove = 7,
    PlayerJoin = 8,
    PlayerLeave = 9,
    PlayerBorn = 10,
    PlayerDead = 11,
    Play = 12,
    Spectate = 13,
    Finish = 14,
    ChatMessage = 15,
  };
};

#endif /* PACKET_OUTPUT_PACKET_TYPES_HPP */
