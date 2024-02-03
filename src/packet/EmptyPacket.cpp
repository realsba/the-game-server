// file   : src/packet/EmptyPacket.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "EmptyPacket.hpp"

#include "serialization.hpp"

EmptyPacket::EmptyPacket(uint8_t type) : type(type) { }

void EmptyPacket::format(Buffer& buffer)
{
  serialize(buffer, type);
}