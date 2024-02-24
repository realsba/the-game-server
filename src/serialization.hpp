// file   : src/serialization.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PACKET_SERIALIZATION_HPP
#define THEGAME_PACKET_SERIALIZATION_HPP

#include "src/types.hpp"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/endian/conversion.hpp>

#include <stdexcept>
#include <string>

namespace beast = boost::beast;

template <typename T>
void serialize(Buffer& buffer, const T& data)
{
  T value = boost::endian::native_to_big(data);
  const auto* ptr = reinterpret_cast<const char*>(&value);
  buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

template <>
inline void serialize<std::string>(Buffer& buffer, const std::string& data)
{
  serialize(buffer, static_cast<uint16_t>(data.length()));
  buffer.insert(buffer.end(), data.begin(), data.end());
}

template <>
inline void serialize<float>(Buffer& buffer, const float& data)
{
  union { float r; uint32_t i; } u{.r = data};
  serialize(buffer, u.i);
}

template <typename T>
T deserialize(beast::flat_buffer& buffer)
{
  T result;

  if (buffer.size() < sizeof(T)) {
    throw std::runtime_error("Not enough result for deserialization");
  }

  std::memcpy(&result, buffer.data().data(), sizeof(T));
  buffer.consume(sizeof(T));

  return boost::endian::big_to_native(result);
}

template <>
inline std::string deserialize<>(beast::flat_buffer& buffer)
{
  auto length = deserialize<uint16_t>(buffer);

  if (buffer.size() < length) {
    throw std::runtime_error("Not enough data for string deserialization");
  }

  std::string result(boost::asio::buffer_cast<const char*>(buffer.data()), length);
  buffer.consume(length);

  return result;
}

#endif /* THEGAME_PACKET_SERIALIZATION_HPP */
