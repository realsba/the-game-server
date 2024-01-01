// file   : geometry/Circle.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GEOMETRY_CIRCLE_HPP
#define THEGAME_GEOMETRY_CIRCLE_HPP

#include "Vec2D.hpp"

class Circle {
public:
  Circle() = default;
  Circle(float x, float y, float r) : position(x, y), radius(r) { }
  Circle(const Vec2D& position, float r) : position(position), radius(r) { }

  Vec2D position;
  float radius {0};
};

#endif /* THEGAME_GEOMETRY_CIRCLE_HPP */
