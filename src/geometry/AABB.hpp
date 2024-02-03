// file   : src/geometry/AABB.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GEOMETRY_AABB_HPP
#define THEGAME_GEOMETRY_AABB_HPP

#include "Vec2D.hpp"

class AABB {
public:
  AABB() = default;
  AABB(float ax, float ay, float bx, float by);
  AABB(const Vec2D& a, const Vec2D& b);

  void normalize();
  void translate(const Vec2D& point);
  [[nodiscard]] bool contain(const Vec2D& point) const;
  [[nodiscard]] float width() const;
  [[nodiscard]] float height() const;

  Vec2D a;
  Vec2D b;
};

#endif /* THEGAME_GEOMETRY_AABB_HPP */
