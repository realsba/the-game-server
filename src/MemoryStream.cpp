// file   : MemoryStream.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "MemoryStream.hpp"

#include "FileIOError.hpp"

#include <netinet/in.h>
#include <algorithm>
#include <fstream>
#include <cerrno>

MemoryStream::MemoryStream(size_t blockSize)  noexcept :
  m_blockSize((blockSize + DefBlockSize - 1) & ~(DefBlockSize - 1))
{ }

MemoryStream::MemoryStream(const MemoryStream& ms) :
  m_blockSize(ms.m_blockSize),
  m_length(ms.m_length),
  m_posG(ms.m_posG),
  m_posP(ms.m_posP)
{
  if (m_length != 0) {
    reserve(m_length);
    ::memcpy(static_cast<char*>(m_data), ms.m_data, m_length);
  }
}

MemoryStream::MemoryStream(const std::string& str) :
  m_blockSize(DefBlockSize)
{
  m_length = str.length();
  if (m_length != 0) {
    reserve(m_length);
    ::memcpy(static_cast<char*>(m_data), str.data(), m_length);
  }
}

MemoryStream::~MemoryStream()
{
  ::free(m_data);
}

MemoryStream& MemoryStream::operator=(const MemoryStream& ms)
{
  // protect against invalid self-assignment
  if (this != &ms) {
    m_blockSize = ms.m_blockSize;
    m_length    = ms.m_length;
    m_posG      = ms.m_posG;
    m_posP      = ms.m_posP;
    if (m_length != 0) {
      reserve(m_length);
      ::memcpy(static_cast<char*>(m_data), ms.m_data, m_length);
    }
  }

  return *this;
}

void MemoryStream::clear()
{
  m_posG    = 0;
  m_posP    = 0;
  m_length  = 0;
}

void MemoryStream::compress()
{
  size_t pos = std::min(m_posG, m_posP);

  if (pos > 0) {
    m_length -= pos;
    ::memmove(m_data, static_cast<char*>(m_data) + pos, m_length);
    m_posP -= pos;
    m_posG -= pos;
  }
}

void MemoryStream::reserve(size_t size)
{
  size_t mask = m_blockSize - 1;
  size_t capacity = (size + mask) & ~mask;
  if (capacity > m_capacity) {
    void* p = ::realloc(m_data, capacity);
    if (p == nullptr && size != 0) {
      throw std::runtime_error("MemoryStream bad realloc");
    }
    m_capacity = capacity;
    m_data = p;
  }
}

void MemoryStream::resize(size_t size)
{
  size_t mask = m_blockSize - 1;
  size_t capacity = (size + mask) & ~mask;
  if (capacity != m_capacity) {
    void* p = ::realloc(m_data, capacity);
    if (p == nullptr && size != 0) {
      throw std::runtime_error("MemoryStream bad realloc");
    }
    m_capacity = capacity;
    m_data = p;
  }
  m_length = size;
  if (m_posG > m_length) {
    m_posG = m_length;
  }
  if (m_posP > m_length) {
    m_posP = m_length;
  }
}

size_t MemoryStream::seekG(ssize_t pos, Direction dir)
{
  size_t prev = m_posG;
  if (dir == Direction::Current) {
    pos += static_cast<ssize_t>(m_posG);
  } else if (dir == Direction::End) {
    pos += static_cast<ssize_t>(m_length);
  }

  if (pos <= 0) {
    m_posG = 0;
  } else {
    if (static_cast<size_t>(pos) > m_capacity) {
      reserve(pos);
    }
    m_posG = pos;
    if (m_posG > m_length) {
      m_length = m_posG;
    }
  }

  return prev;
}

size_t MemoryStream::seekP(ssize_t pos, Direction dir)
{
  size_t prev = m_posP;
  if (dir == Direction::Current) {
    pos += static_cast<ssize_t>(m_posP);
  } else if (dir == Direction::End) {
    pos += static_cast<ssize_t>(m_length);
  }

  if (pos <= 0) {
    m_posP = 0;
  } else {
    if (static_cast<size_t>(pos) > m_capacity) {
      reserve(pos);
    }
    m_posP = pos;
    if (m_posP > m_length) {
      m_length = m_posP;
    }
  }

  return prev;
}

void MemoryStream::peek(void* data, size_t size) const
{
  if (size == 0) {
    return;
  }
  if (m_posG + size > m_length) {
    throw std::runtime_error("MemoryStream read error");
  }
  ::memcpy(data, static_cast<char*>(m_data) + m_posG, size);
}

void MemoryStream::read(void* data, size_t size)
{
  if (size == 0) {
    return;
  }
  if (m_posG + size > m_length) {
    throw std::runtime_error("MemoryStream read error");
  }
  ::memcpy(data, static_cast<char*>(m_data) + m_posG, size);
  m_posG += size;
}

void MemoryStream::write(const void* data, size_t size)
{
  if (size == 0) {
    return;
  }
  size_t len = m_posP + size;
  if (len > m_capacity) {
    reserve(len);
  }
  ::memcpy(static_cast<char*>(m_data) + m_posP, data, size);
  m_posP += size;
  if (m_posP > m_length) {
    m_length = m_posP;
  }
}

int8_t MemoryStream::peekInt8() const
{
  int8_t v;
  peek(&v, sizeof(v));
  return v;
}

uint8_t MemoryStream::peekUInt8() const
{
  uint8_t v;
  peek(&v, sizeof(v));
  return v;
}

int16_t MemoryStream::peekInt16() const
{
  int16_t v;
  peek(&v, sizeof(v));
  return be16toh(v);
}

uint16_t MemoryStream::peekUInt16() const
{
  uint16_t v;
  peek(&v, sizeof(v));
  return be16toh(v);
}

int32_t MemoryStream::peekInt32() const
{
  int32_t v;
  peek(&v, sizeof(v));
  return be32toh(v);
}

uint32_t MemoryStream::peekUInt32() const
{
  uint32_t v;
  peek(&v, sizeof(v));
  return be32toh(v);
}

int64_t MemoryStream::peekInt64() const
{
  int64_t v;
  peek(&v, sizeof(v));
  return be64toh(v);
}

uint64_t MemoryStream::peekUInt64() const
{
  uint64_t v;
  peek(&v, sizeof(v));
  return be64toh(v);
}

float MemoryStream::peekFloat() const
{
  union { float r; uint32_t i; } u{};
  peek(&u.i, sizeof(u.i));
  u.i = be32toh(u.i);
  return u.r;
}

double MemoryStream::peekDouble() const
{
  union { double r; uint64_t i; } u{};
  peek(&u.i, sizeof(u.i));
  u.i = be64toh(u.i);
  return u.r;
}

std::string MemoryStream::peekString() const
{
  size_t length = peekUInt16();
  size_t pos = m_posG + 2;
  if (pos + length > m_length) {
    throw std::runtime_error("MemoryStream read error");
  }
  std::string s(static_cast<char*>(m_data) + pos, length);
  return s;
}

int8_t MemoryStream::readInt8()
{
  int8_t v;
  read(&v, sizeof(v));
  return v;
}

uint8_t MemoryStream::readUInt8()
{
  uint8_t v;
  read(&v, sizeof(v));
  return v;
}

int16_t MemoryStream::readInt16()
{
  int16_t v;
  read(&v, sizeof(v));
  return be16toh(v);
}

uint16_t MemoryStream::readUInt16()
{
  uint16_t v;
  read(&v, sizeof(v));
  return be16toh(v);
}

int32_t MemoryStream::readInt32()
{
  int32_t v;
  read(&v, sizeof(v));
  return be32toh(v);
}

uint32_t MemoryStream::readUInt32()
{
  uint32_t v;
  read(&v, sizeof(v));
  return be32toh(v);
}

int64_t MemoryStream::readInt64()
{
  int64_t v;
  read(&v, sizeof(v));
  return be64toh(v);
}

uint64_t MemoryStream::readUInt64()
{
  uint64_t v;
  read(&v, sizeof(v));
  return be64toh(v);
}

float MemoryStream::readFloat()
{
  union { float r; uint32_t i; } u{};
  read(&u.i, sizeof(u.i));
  u.i = be32toh(u.i);
  return u.r;
}

double MemoryStream::readDouble()
{
  union { double r; uint64_t i; } u{};
  read(&u.i, sizeof(u.i));
  u.i = be64toh(u.i);
  return u.r;
}

std::string MemoryStream::readString()
{
  size_t length = readUInt16();
  if (m_posG + length > m_length) {
    throw std::runtime_error("MemoryStream read error");
  }
  std::string s(static_cast<char*>(m_data) + m_posG, length);
  m_posG += length;
  return s;
}

void MemoryStream::writeInt8(int8_t v)
{
  write(&v, sizeof(v));
}

void MemoryStream::writeUInt8(uint8_t v)
{
  write(&v, sizeof(v));
}

void MemoryStream::writeInt16(int16_t v)
{
  v = htobe16(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeUInt16(uint16_t v)
{
  v = htobe16(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeInt32(int32_t v)
{
  v = htobe32(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeUInt32(uint32_t v)
{
  v = htobe32(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeInt64(int64_t v)
{
  v = htobe64(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeUInt64(uint64_t v)
{
  v = htobe64(v);
  write(&v, sizeof(v));
}

void MemoryStream::writeFloat(float v)
{
  union { float r; uint32_t i; } u{};
  u.r = v;
  u.i = htobe32(u.i);
  write(&u.i, sizeof(u.i));
}

void MemoryStream::writeDouble(double v)
{
  union { double r; uint64_t i; } u{};
  u.r = v;
  u.i = htobe64(u.i);
  write(&u.i, sizeof(u.i));
}

void MemoryStream::writeString(const std::string& v)
{
  size_t len = v.length();
  if (len > 0xFFFF) {
    len = 0xFFFF;
  }
  writeUInt16(static_cast<uint16_t>(len));
  write(v.c_str(), len);
}

void MemoryStream::loadFromFile(const std::string& filename)
{
  std::ifstream fs(filename.c_str(), std::ios_base::in | std::ios_base::binary);
  if (!fs.is_open()) {
    throw FileIoError(filename, errno);
  }

  fs.exceptions (std::ios::eofbit | std::ios::failbit | std::ios::badbit);

  fs.seekg(0, std::ios_base::end);
  size_t len = fs.tellg();
  if (len == 0) {
    return;
  }

  reserve(len);

  fs.seekg(0, std::ios_base::beg);
  fs.read(static_cast<char*>(m_data), static_cast<ssize_t>(len));
  fs.close();

  m_posG    = 0;
  m_posP    = len;
  m_length  = len;
}

void MemoryStream::saveToFile(const std::string& filename) const
{
  std::ofstream fs(filename.c_str(), std::ios_base::out | std::ios_base::binary);
  if (!fs.is_open()) {
    throw FileIoError(filename, errno);
  }
  fs.write(static_cast<char*>(m_data), static_cast<ssize_t>(m_length));
  fs.close();
}

std::ostream& operator<<(std::ostream& os, const MemoryStream& obj)
{
  char hex_val[] = "0123456789ABCDEF";

  os << "blockSize=" << obj.m_blockSize << ", capacity=" << obj.m_capacity
    << ", posG=" << obj.m_posG << ", posP=" << obj.m_posP << ", length=" << obj.m_length << ", data=";
  auto* data = static_cast<uint8_t*>(obj.m_data);
  for (auto *it = data, *end = data + obj.m_length; it != end; ++it) {
    auto ch = *it;
    os << hex_val[ch >> 4] << hex_val[ch & 15];
  }

  return os;
}
