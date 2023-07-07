// file   : packet/EmptyPacket.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "EmptyPacket.hpp"

#include "src/MemoryStream.hpp"

EmptyPacket::EmptyPacket(uint8_t type) : type(type) { }

void EmptyPacket::format(MemoryStream& ms)
{
  prepareHeader(ms);
  writeHeader(ms, type);
}
