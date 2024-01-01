// file   : packet/PacketPlay.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketPlay.hpp"

#include "OutputPacketTypes.hpp"
#include "src/Player.hpp"

PacketPlay::PacketPlay(const Player& player)
  : m_player(player)
{
}

void PacketPlay::format(std::vector<char>& buffer)
{
  serialize(buffer, static_cast<uint8_t>(OutputPacketTypes::Play));
  serialize(buffer, m_player.getId());
  const auto& position = m_player.getPosition();
  serialize(buffer, static_cast<uint16_t>(position.x));
  serialize(buffer, static_cast<uint16_t>(position.y));
  serialize(buffer, m_player.getMaxMass());
}
