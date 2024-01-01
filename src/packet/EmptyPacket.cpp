// file   : packet/EmptyPacket.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "EmptyPacket.hpp"

EmptyPacket::EmptyPacket(uint8_t type) : type(type) { }

void EmptyPacket::format(std::vector<char>& buffer)
{
  serialize(buffer, type);
}