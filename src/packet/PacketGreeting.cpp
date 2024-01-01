// file   : packet/PacketGreeting.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketGreeting.hpp"

#include "OutputPacketTypes.hpp"

PacketGreeting::PacketGreeting(std::string sid) : sid(std::move(sid)) { }

void PacketGreeting::format(std::vector<char>& buffer)
{
  serialize(buffer, static_cast<uint8_t>(OutputPacketTypes::Greeting));
  serialize(buffer, sid);
}
