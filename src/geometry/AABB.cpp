// file   : src/geometry/AABB.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "AABB.hpp"

#include <utility>

AABB::AABB(float ax, float ay, float bx, float by)
  : a(ax, ay), b(bx, by)
{ }

AABB::AABB(const Vec2D& a, const Vec2D& b)
  : a(a), b(b)
{ }

void AABB::normalize()
{
  if (a.x > b.x) {
    std::swap(a.x, b.x);
  }
  if (a.y > b.y) {
    std::swap(a.y, b.y);
  }
}

void AABB::translate(const Vec2D& point)
{
  a += point;
  b += point;
}

bool AABB::contain(const Vec2D& point) const
{
  return point.x >= a.x && point.x <= b.x && point.y >= a.y && point.y <= b.y;
}

float AABB::width() const
{
  return b.x - a.x;
}

float AABB::height() const
{
  return b.y - a.y;
}
