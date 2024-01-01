// file      : AsioFormatter.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ASIO_FORMATTER_HPP
#define THEGAME_ASIO_FORMATTER_HPP

#include <fmt/format.h>

#include <boost/asio/ip/tcp.hpp>

// Provide a specializations for fmt::format
namespace fmt {

template <>
struct formatter<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>, char> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>& ep, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}:{}", ep.address().to_string(), ep.port());
  }
};

} // namespace fmt

#endif /* THEGAME_ASIO_FORMATTER_HPP */
