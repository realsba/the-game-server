// file   : packet/PacketGreeting.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketGreeting.hpp"

#include "OutputPacketTypes.hpp"
#include "src/MemoryStream.hpp"

PacketGreeting::PacketGreeting() { }

PacketGreeting::PacketGreeting(const std::string& sid) : sid(sid) { }

void PacketGreeting::format(MemoryStream& ms)
{
  prepareHeader(ms);
  ms.writeString(sid);
  writeHeader(ms, OutputPacketTypes::Greeting);
}
