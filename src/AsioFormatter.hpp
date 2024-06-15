// file      : src/AsioFormatter.cpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ASIO_FORMATTER_HPP
#define THEGAME_ASIO_FORMATTER_HPP

#include <fmt/core.h>

#include <boost/asio/ip/tcp.hpp>

// Provide a specializations for fmt::format
namespace fmt {

template <>
struct formatter<asio::ip::basic_endpoint<asio::ip::tcp>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const asio::ip::basic_endpoint<asio::ip::tcp>& ep, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}:{}", ep.address().to_string(), ep.port());
  }
};

} // namespace fmt

#endif /* THEGAME_ASIO_FORMATTER_HPP */
