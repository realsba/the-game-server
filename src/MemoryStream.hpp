// file   : MemoryStream.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef MEMORY_STREAM_HPP
#define MEMORY_STREAM_HPP

#include <ostream>
#include <string>
#include <cstdint>

class MemoryStream {
public:
  enum class Direction {Begin, Current, End};

  explicit MemoryStream(size_t block_size=DefBlockSize) noexcept;
  MemoryStream(const MemoryStream& ms);
  explicit MemoryStream(const std::string& str);

  virtual ~MemoryStream();

  MemoryStream& operator=(const MemoryStream& ms);

  void clear();
  void compress();
  void reserve(size_t size);
  void resize(size_t size);
  size_t seekG(ssize_t pos, Direction dir);
  size_t seekP(ssize_t pos, Direction dir);

  void peek(void* data, size_t size) const;
  void read(void* data, size_t size);
  void write(const void* data, size_t size);

  int8_t peekInt8() const;
  uint8_t peekUInt8() const;
  int16_t peekInt16() const;
  uint16_t peekUInt16() const;
  int32_t peekInt32() const;
  uint32_t peekUInt32() const;
  int64_t peekInt64() const;
  uint64_t peekUInt64() const;
  float peekFloat() const;
  double peekDouble() const;
  std::string peekString() const;

  int8_t readInt8();
  uint8_t readUInt8();
  int16_t readInt16();
  uint16_t readUInt16();
  int32_t readInt32();
  uint32_t readUInt32();
  int64_t readInt64();
  uint64_t readUInt64();
  float readFloat();
  double readDouble();
  std::string readString();

  void writeInt8(int8_t v);
  void writeUInt8(uint8_t v);
  void writeInt16(int16_t v);
  void writeUInt16(uint16_t v);
  void writeInt32(int32_t v);
  void writeUInt32(uint32_t v);
  void writeInt64(int64_t v);
  void writeUInt64(uint64_t v);
  void writeFloat(float v);
  void writeDouble(double v);
  void writeString(const std::string& v);

  void loadFromFile(const std::string& filename);
  void saveToFile(const std::string& filename) const;

  void* data() const;
  void* dataG() const;
  void* dataP() const;
  size_t blockSize() const;
  size_t capacity() const;
  size_t length() const;
  size_t tellG() const;
  size_t tellP() const;
  size_t availableForRead() const;
  size_t availableForWrite() const;

protected:
  enum {
    DefBlockSize = 256
  };

  void*   m_data {nullptr};
  size_t  m_blockSize {0};
  size_t  m_capacity {0};
  size_t  m_length {0};
  size_t  m_posG {0};
  size_t  m_posP {0};

private:
  friend std::ostream& operator<<(std::ostream& os, const MemoryStream& obj);
};

inline void* MemoryStream::data() const
{
  return m_data;
}

inline void* MemoryStream::dataG() const
{
  return static_cast<char*>(m_data) + m_posG;
}

inline void* MemoryStream::dataP() const
{
  return static_cast<char*>(m_data) + m_posP;
}

inline size_t MemoryStream::blockSize() const
{
  return m_blockSize;
}

inline size_t MemoryStream::capacity() const
{
  return m_capacity;
}

inline size_t MemoryStream::length() const
{
  return m_length;
}

inline size_t MemoryStream::tellG() const
{
  return m_posG;
}

inline size_t MemoryStream::tellP() const
{
  return m_posP;
}

inline size_t MemoryStream::availableForRead() const
{
  return m_length > m_posG ? m_length - m_posG : 0;
}

inline size_t MemoryStream::availableForWrite() const
{
  return m_capacity - m_posP;
}

std::ostream& operator<<(std::ostream& os, const MemoryStream& obj);

#endif /* MEMORY_STREAM_HPP  */
