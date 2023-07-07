// file   : packet/PacketPlay.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketPlay.hpp"

#include "OutputPacketTypes.hpp"
#include "src/MemoryStream.hpp"
#include "src/Player.hpp"

PacketPlay::PacketPlay(const Player& player) : m_player(player) { }

void PacketPlay::format(MemoryStream& ms)
{
  prepareHeader(ms);
  ms.writeUInt32(m_player.getId());
  const auto& position = m_player.getPosition();
  ms.writeUInt16(position.x);
  ms.writeUInt16(position.y);
  ms.writeUInt32(m_player.getMaxMass());
  writeHeader(ms, OutputPacketTypes::Play);
}
