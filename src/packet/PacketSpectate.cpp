// file   : packet/PacketSpectate.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketSpectate.hpp"

#include "OutputPacketTypes.hpp"
#include "src/MemoryStream.hpp"
#include "src/Player.hpp"

PacketSpectate::PacketSpectate(const Player& player) : m_player(player) { }

void PacketSpectate::format(MemoryStream& ms)
{
  prepareHeader(ms);
  ms.writeUInt32(m_player.getId());
  const auto& position = m_player.getPosition();
  ms.writeUInt16(position.x);
  ms.writeUInt16(position.y);
  ms.writeUInt32(m_player.getMaxMass());
  writeHeader(ms, OutputPacketTypes::Spectate);
}
