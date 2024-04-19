// file   : src/OutgoingPacket.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "OutgoingPacket.hpp"

#include "serialization.hpp"

#include "Player.hpp"

namespace OutgoingPacket {

void serializePong(Buffer& buffer)
{
  serialize(buffer, Type::Pong);
}

void serializeGreeting(Buffer& buffer, const std::string& sid)
{
  serialize(buffer, Type::Greeting);
  serialize(buffer, sid);
}

void serializeRoom(Buffer& buffer)
{
}

void serializeFrame(Buffer& buffer)
{
}

void serializeLeaderboard(Buffer& buffer, const std::vector<PlayerPtr>& items, size_t limit)
{
  serialize(buffer, Type::Leaderboard);
  auto count = static_cast<uint8_t>(std::min(items.size(), limit));
  serialize(buffer, count);
  auto endIt = items.begin() + count;
  for (auto it = items.begin(); it != endIt; ++it) {
    const auto& player = *it;
    serialize(buffer, player->getId());
    serialize(buffer, player->getMass());
  }
}

void serializePlayer(Buffer& buffer, const Player& player)
{
  serialize(buffer, Type::Player);
  serialize(buffer, player.getId());
  serialize(buffer, player.getName());
}

void serializePlayerRemove(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::PlayerRemove);
  serialize(buffer, playerId);
}

void serializePlayerJoin(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::PlayerJoin);
  serialize(buffer, playerId);
}

void serializePlayerLeave(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::PlayerLeave);
  serialize(buffer, playerId);
}

void serializePlayerBorn(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::PlayerBorn);
  serialize(buffer, playerId);
}

void serializePlayerDead(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::PlayerDead);
  serialize(buffer, playerId);
}

void serializePlay(Buffer& buffer, const Player& player)
{
  serialize(buffer, Type::Play);
  serialize(buffer, player.getId());
  const auto& position = player.getPosition();
  serialize(buffer, static_cast<uint16_t>(position.x));
  serialize(buffer, static_cast<uint16_t>(position.y));
  serialize(buffer, player.getMaxMass());
}

void serializeSpectate(Buffer& buffer, const Player& player)
{
  serialize(buffer, Type::Spectate);
  serialize(buffer, player.getId());
  const auto& position = player.getPosition();
  serialize(buffer, static_cast<uint16_t>(position.x));
  serialize(buffer, static_cast<uint16_t>(position.y));
  serialize(buffer, player.getMaxMass());
}

void serializeFinish(Buffer& buffer)
{
  serialize(buffer, Type::Finish);
}

void serializeChatMessage(Buffer& buffer, uint32_t playerId, const std::string& text)
{
  serialize(buffer, Type::ChatMessage);
  serialize(buffer, playerId);
  serialize(buffer, text);
}

void serializeChangeTargetPlayer(Buffer& buffer, uint32_t playerId)
{
  serialize(buffer, Type::ChangeTargetPlayer);
  serialize(buffer, playerId);
}

} // namespace OutgoingPacket