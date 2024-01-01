// file   : packet/Packet.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_PACKET_HPP
#define THEGAME_PACKET_PACKET_HPP

#include <cstdint>
#include <cstddef>

#include <vector>
#include <string>
#include <boost/endian/conversion.hpp>

class Cell;

// TODO: remove
#include <memory>
using Buffer = std::vector<char>;
using BufferPtr = std::shared_ptr<Buffer>;

template<typename T>
void serialize(std::vector<char>& buffer, const T& data)
{
  T value = boost::endian::native_to_big(data);
  const auto* ptr = reinterpret_cast<const char*>(&value);
  buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

template<>
inline void serialize<std::string>(std::vector<char>& buffer, const std::string& data)
{
  serialize(buffer, static_cast<uint16_t>(data.length()));
  buffer.insert(buffer.end(), data.begin(), data.end());
}

inline void serialize(std::vector<char>& buffer, float data)
{
  union { float r; uint32_t i; } u{.r = data};
  serialize(buffer, u.i);
}

#include <stdexcept>

#include <boost/beast/core.hpp>

namespace beast = boost::beast;

class Deserializer {
public:
  explicit Deserializer(beast::flat_buffer& buffer) : m_buffer(buffer) {}

  template<typename T>
  void deserialize(T& result)
  {
    if (m_buffer.size() < sizeof(T)) {
      throw std::runtime_error("Not enough result for deserialization");
    }

    std::memcpy(&result, m_buffer.data().data(), sizeof(T));
    m_buffer.consume(sizeof(T));
    result = boost::endian::big_to_native(result);
  }

  void deserialize(std::string& result)
  {
    uint16_t length;
    deserialize(length);

    if (m_buffer.size() < length) {
      throw std::runtime_error("Not enough data for string deserialization");
    }

    result.assign(boost::asio::buffer_cast<const char*>(m_buffer.data()), length);
    m_buffer.consume(length);
  }

  std::string readString()
  {
    std::string result;
    deserialize(result);
    return result;
  }

  uint8_t readUInt8()
  {
    uint8_t result;
    deserialize(result);
    return result;
  }

  uint32_t readUInt32()
  {
    uint32_t result;
    deserialize(result);
    return result;
  }

  int16_t readInt16()
  {
    int16_t result;
    deserialize(result);
    return result;
  }

private:
  beast::flat_buffer& m_buffer;
};

#endif /* THEGAME_PACKET_PACKET_HPP */
