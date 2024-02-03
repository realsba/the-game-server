// file      : src/geometry/formatter.hpp
// author    : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GEOMETRY_FORMATTER_HPP
#define THEGAME_GEOMETRY_FORMATTER_HPP

#include <fmt/core.h>

#include "AABB.hpp"

// Provide a specializations for fmt::format
namespace fmt {

  template <>
  struct formatter<AABB> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const AABB& aabb, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "[{}; {}]", aabb.a, aabb.b);
    }
  };

  template <>
  struct formatter<Vec2D> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Vec2D& vec, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
    }
  };

} // namespace fmt

#endif /* THEGAME_GEOMETRY_FORMATTER_HPP */
