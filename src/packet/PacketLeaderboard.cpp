// file   : packet/PacketLeaderboard.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketLeaderboard.hpp"

#include "OutputPacketTypes.hpp"
#include "serialization.hpp"

#include "src/Player.hpp"

PacketLeaderboard::PacketLeaderboard(const std::vector<Player*>& items, size_t limit)
  : m_items(items)
  , m_limit(limit)
{
}

void PacketLeaderboard::format(Buffer& buffer)
{
  serialize(buffer, static_cast<uint8_t>(OutputPacketTypes::Leaderboard));
  auto count = static_cast<uint8_t>(std::min(m_items.size(), m_limit));
  serialize(buffer, count);
  auto endIt = m_items.begin() + count;
  for (auto it = m_items.begin(); it != endIt; ++it) {
    const auto* player = *it;
    serialize(buffer, player->getId());
    serialize(buffer, player->getMass());
  }
}
