// file   : src/geometry/geometry.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GEOMETRY_GEOMETRY_HPP
#define THEGAME_GEOMETRY_GEOMETRY_HPP

class Circle;
class Vec2D;
class AABB;

namespace geometry {

bool intersects(const Circle& circle, const Vec2D& p);
bool intersects(const Circle& a, const Circle& b);
bool intersects(const AABB& box, const Vec2D& p);
bool intersects(const AABB& box, const Circle& circle);

float distance(const Vec2D& a, const Vec2D& b);
float squareDistance(const Vec2D& a, const Vec2D& b);

} // namespace geometry

#endif /* THEGAME_GEOMETRY_GEOMETRY_HPP */
