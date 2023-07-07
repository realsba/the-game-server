// file   : packet/Packet.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef PACKET_PACKET_HPP
#define PACKET_PACKET_HPP

#include <cstdint>
#include <cstddef>

class MemoryStream;

class Packet {
public:
  void prepareHeader(MemoryStream& ms);
  void writeHeader(MemoryStream& ms, uint8_t type);

protected:
  enum {
    HeaderSize = 1
  };

  size_t m_pos {0};
  bool m_started {false};
};

#endif /* PACKET_PACKET_HPP */
