// file   : geometry/geometry.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "geometry.hpp"

#include "Circle.hpp"
#include "Vec2D.hpp"
#include "AABB.hpp"

#include <cmath>

namespace geometry {

bool intersects(const Circle& circle, const Vec2D& p)
{
  float a = p.x - circle.position.x;
  float b = p.y - circle.position.y;
  return a * a + b * b <= circle.radius * circle.radius;
}

bool intersects(const Circle& a, const Circle& b)
{
  return squareDistance(a.position, b.position) <= (a.radius + b.radius) * (a.radius + b.radius);
}

bool intersects(const AABB& box, const Vec2D& p)
{
  return p.x >= box.a.x && p.x <= box.b.x && p.y >= box.a.y && p.y <= box.b.y;
}

bool intersects(const AABB& box, const Circle& circle)
{
  if (circle.position.x < box.a.x) {
    if (box.a.x - circle.position.x > circle.radius) {
      return false;
    }
    if (circle.position.y < box.a.y) {
      return  box.a.y - circle.position.y <= circle.radius;
    }
    if (circle.position.y > box.b.y) {
      return circle.position.y - box.b.y <= circle.radius;
    }
  } else if (circle.position.x > box.b.x) {
    if (circle.position.x - box.b.x > circle.radius) {
      return false;
    }
    if (circle.position.y < box.a.y) {
      return  box.a.y - circle.position.y <= circle.radius;
    }
    if (circle.position.y > box.b.y) {
      return circle.position.y - box.b.y <= circle.radius;
    }
  } else if (circle.position.y < box.a.y) {
    return box.a.y - circle.position.y <= circle.radius;
  } else if (circle.position.y > box.b.y) {
    return circle.position.y - box.b.y <= circle.radius;
  }
  return true;
}

float distance(const Vec2D& p1, const Vec2D& p2)
{
  float a = p1.x - p2.x;
  float b = p1.y - p2.y;
  return std::sqrt(a * a + b * b);
}

float squareDistance(const Vec2D& p1, const Vec2D& p2)
{
  float a = p1.x - p2.x;
  float b = p1.y - p2.y;
  return a * a + b * b;
}

} // namespace geometry
