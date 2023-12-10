// file   : packet/Packet.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Packet.hpp"

#include "src/MemoryStream.hpp"
#include "src/Logger.hpp"

void Packet::prepareHeader(MemoryStream& ms)
{
  if (m_started) {
    throw std::logic_error("Packet already started.");
  }
  m_started = true;
  m_pos = ms.seekP(HeaderSize, MemoryStream::Direction::Current);
}

void Packet::writeHeader(MemoryStream& ms, uint8_t type)
{
  if (!m_started) {
    throw std::logic_error("Packet must be started.");
  }
  size_t pos = ms.seekP(static_cast<ssize_t>(m_pos), MemoryStream::Direction::Begin);
  m_pos += HeaderSize;
  if (pos < m_pos) {
    throw std::logic_error("Violation of writing order in the stream");
  }
  ms.writeUInt8(type);
  ms.seekP(static_cast<ssize_t>(pos), MemoryStream::Direction::Begin);
  m_started = false;
}
