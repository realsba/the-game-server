// file   : packet/PacketGreeting.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PacketGreeting.hpp"

#include <utility>

#include "OutputPacketTypes.hpp"
#include "src/MemoryStream.hpp"

PacketGreeting::PacketGreeting(std::string sid) : sid(std::move(sid)) { }

void PacketGreeting::format(MemoryStream& ms)
{
  prepareHeader(ms);
  ms.writeString(sid);
  writeHeader(ms, OutputPacketTypes::Greeting);
}
