// file   : packet/PacketLeaderboard.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketLeaderboard.hpp"

#include "OutputPacketTypes.hpp"
#include "src/MemoryStream.hpp"
#include "src/Player.hpp"

#include <algorithm>

void PacketLeaderboard::format(MemoryStream& ms, const std::vector<Player*>& items, uint32_t max)
{
  prepareHeader(ms);
  uint32_t count = std::min(static_cast<uint32_t>(items.size()), max);
  ms.writeUInt8(count);
  for (const Player* player : items) {
    if (count == 0) {
      break;
    }
    ms.writeUInt32(player->getId());
    ms.writeUInt32(player->getMass());
    --count;
  }
  writeHeader(ms, OutputPacketTypes::Leaderboard);
}
