// file   : tests/geometry/Test_geometry.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <catch2/catch_test_macros.hpp>

#include "../../src/geometry/geometry.hpp"
#include "../../src/geometry/AABB.hpp"
#include "../../src/geometry/Circle.hpp"

TEST_CASE("Circle and Point Intersection", "[geometry]")
{
  Circle circle({0, 0}, 5);
  Vec2D pointInside(3, 4);
  Vec2D pointOutside(10, 10);

  REQUIRE(geometry::intersects(circle, pointInside) == true);
  REQUIRE(geometry::intersects(circle, pointOutside) == false);
}

TEST_CASE("Circle and Circle Intersection", "[geometry]")
{
  Circle circle1({0, 0}, 5);
  Circle circle2({8, 0}, 5);
  Circle circle3({15, 0}, 5);

  REQUIRE(geometry::intersects(circle1, circle2) == true);
  REQUIRE(geometry::intersects(circle1, circle3) == false);
}

TEST_CASE("AABB and Point Intersection", "[geometry]")
{
  AABB box({0, 0}, {10, 10});
  Vec2D pointInside(5, 5);
  Vec2D pointOutside(15, 15);

  REQUIRE(geometry::intersects(box, pointInside) == true);
  REQUIRE(geometry::intersects(box, pointOutside) == false);
}

TEST_CASE("AABB and Circle Intersection", "[geometry]")
{
  AABB box({0, 0}, {10, 10});
  Circle circleInside({5, 5}, 3);
  Circle circleOutside({15, 15}, 3);

  REQUIRE(geometry::intersects(box, circleInside) == true);
  REQUIRE(geometry::intersects(box, circleOutside) == false);
}

TEST_CASE("Distance between two points", "[geometry]")
{
  Vec2D point1(0, 0);
  Vec2D point2(3, 4);

  REQUIRE(geometry::distance(point1, point2) == 5.0);
}

TEST_CASE("Square distance between two points", "[geometry]")
{
  Vec2D point1(0, 0);
  Vec2D point2(3, 4);

  REQUIRE(geometry::squareDistance(point1, point2) == 25.0);
}
