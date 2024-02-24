// file   : src/OutgoingPacket.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "OutgoingPacket.hpp"

#include "serialization.hpp"

#include "Player.hpp"

namespace OutgoingPacket {

void serializePong(Buffer& buffer)
{
  serialize(buffer, OutgoingPacket::Type::Pong);
}

void serializeGreeting(Buffer& buffer, const std::string& sid)
{
  serialize(buffer, OutgoingPacket::Type::Greeting);
  serialize(buffer, sid);
}

void serializeRoom(Buffer& buffer)
{
}

void serializeFrame(Buffer& buffer)
{
}

void serializeLeaderboard(Buffer& buffer, const std::vector<Player*>& items, size_t limit)
{
  serialize(buffer, OutgoingPacket::Type::Leaderboard);
  auto count = static_cast<uint8_t>(std::min(items.size(), limit));
  serialize(buffer, count);
  auto endIt = items.begin() + count;
  for (auto it = items.begin(); it != endIt; ++it) {
    const auto* player = *it;
    serialize(buffer, player->getId());
    serialize(buffer, player->getMass());
  }
}

void serializePlayer(Buffer& buffer, const Player& player)
{
  serialize(buffer, OutgoingPacket::Type::Player);
  serialize(buffer, player.getId());
  serialize(buffer, player.name);
}

void serializePlayerRemove(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, OutgoingPacket::Type::PlayerRemove);
  serialize(buffer, playerId);
}

void serializePlayerJoin(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, OutgoingPacket::Type::PlayerJoin);
  serialize(buffer, playerId);
}

void serializePlayerLeave(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, OutgoingPacket::Type::PlayerLeave);
  serialize(buffer, playerId);
}

void serializePlayerBorn(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, OutgoingPacket::Type::PlayerBorn);
  serialize(buffer, playerId);
}

void serializePlayerDead(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, OutgoingPacket::Type::PlayerDead);
  serialize(buffer, playerId);
}

void serializePlay(Buffer& buffer, const Player& player)
{
  serialize(buffer, OutgoingPacket::Type::Play);
  serialize(buffer, player.getId());
  const auto& position = player.getPosition();
  serialize(buffer, static_cast<uint16_t>(position.x));
  serialize(buffer, static_cast<uint16_t>(position.y));
  serialize(buffer, player.getMaxMass());
}

void serializeSpectate(Buffer& buffer, const Player& player)
{
  serialize(buffer, OutgoingPacket::Type::Spectate);
  serialize(buffer, player.getId());
  const auto& position = player.getPosition();
  serialize(buffer, static_cast<uint16_t>(position.x));
  serialize(buffer, static_cast<uint16_t>(position.y));
  serialize(buffer, player.getMaxMass());
}

void serializeFinish(Buffer& buffer)
{
  serialize(buffer, OutgoingPacket::Type::Finish);
}

void serializeChatMessage(Buffer& buffer, uint32_t playerId, const std::string& text)
{
  serialize(buffer, OutgoingPacket::Type::ChatMessage);
  serialize(buffer, playerId);
  serialize(buffer, text);
}

} // namespace OutgoingPacket