// file   : geometry/AABB.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef GEOMETRY_AABB_HPP
#define GEOMETRY_AABB_HPP

#include "Vec2D.hpp"

class AABB {
public:
  AABB();
  AABB(float ax, float ay, float bx, float by);
  AABB(const Vec2D& a, const Vec2D& b);

  void normalize();
  void translate(const Vec2D& point);
  bool contain(const Vec2D& point) const;
  float width() const;
  float height() const;

  Vec2D a;
  Vec2D b;

private:
  friend std::ostream& operator<<(std::ostream& os, const AABB& obj);
};

#endif /* GEOMETRY_AABB_HPP */
