// file   : src/OutgoingPacket.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_OUTGOING_PACKET_HPP
#define THEGAME_OUTGOING_PACKET_HPP

#include "types.hpp"

#include "PlayerFwd.hpp"

namespace OutgoingPacket {

enum class Type : uint8_t {
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
  ChangeTargetPlayer = 16,
};

void serializePong(Buffer& buffer);
void serializeGreeting(Buffer& buffer, const std::string& sid);
void serializeRoom(Buffer& buffer);   // TODO: implement
void serializeFrame(Buffer& buffer);  // TODO: implement
void serializeLeaderboard(Buffer& buffer, const std::vector<PlayerPtr>& items, size_t limit);
void serializePlayer(Buffer& buffer, const Player& player);
void serializePlayerRemove(Buffer& buffer, uint32_t playerId);
void serializePlayerJoin(Buffer& buffer, uint32_t playerId);
void serializePlayerLeave(Buffer& buffer, uint32_t playerId);
void serializePlayerBorn(Buffer& buffer, uint32_t playerId);
void serializePlayerDead(Buffer& buffer, uint32_t playerId);
void serializePlay(Buffer& buffer, const Player& player);
void serializeSpectate(Buffer& buffer, const Player& player);
void serializeFinish(Buffer& buffer);
void serializeChatMessage(Buffer& buffer, uint32_t playerId, const std::string& text);
void serializeChangeTargetPlayer(Buffer& buffer, uint32_t playerId);

} // namespace OutgoingPacket

#endif /* THEGAME_OUTGOING_PACKET_HPP */
