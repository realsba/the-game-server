// file   : tests/geometry/Test_Vec2D.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <catch2/catch_test_macros.hpp>

#include "../../src/geometry/Vec2D.hpp"

TEST_CASE("Vec2D: direction", "[Vec2D]")
{
  Vec2D v;
  CHECK(v.direction() == v);
  CHECK(v.direction() == Vec2D());

  v = Vec2D(100, 0);
  CHECK(v.direction() == Vec2D(1, 0));

  v = Vec2D(-100, 0);
  CHECK(v.direction() == Vec2D(-1, 0));

  v = Vec2D(0, 100);
  CHECK(v.direction() == Vec2D(0, 1));

  v = Vec2D(0, -100);
  CHECK(v.direction() == Vec2D(0, -1));
}

TEST_CASE("Vec2D: length", "[Vec2D]")
{
  Vec2D v(42, 0);
  CHECK(v.length() == 42);
  v = Vec2D(0, 42);
  CHECK(v.length() == 42);
  v = Vec2D(3, 4);
  CHECK(v.length() == 5);
  v = Vec2D(4, 3);
  CHECK(v.length() == 5);
}

TEST_CASE("Vec2D: square length", "[Vec2D]")
{
  Vec2D v(42, 0);
  CHECK(v.squareLength() == 42 * 42);
  v = Vec2D(0, 42);
  CHECK(v.squareLength() == 42 * 42);
  v = Vec2D(3, 4);
  CHECK(v.squareLength() == 5 * 5);
  v = Vec2D(4, 3);
  CHECK(v.squareLength() == 5 * 5);

}

TEST_CASE("Vec2D: normalize", "[Vec2D]")
{
  Vec2D v(42, 0);
  v.normalize();
  CHECK(v == Vec2D(1, 0));
  v = Vec2D(0, 42);
  v.normalize();
  CHECK(v == Vec2D(0, 1));
}

TEST_CASE("Vec2D: zero", "[Vec2D]")
{
  Vec2D v(3, 4);
  v.zero();
  CHECK(v == Vec2D(0, 0));
}

TEST_CASE("Vec2D: unary minus", "[Vec2D]")
{
  Vec2D v(3, 4);
  v = -v;
  CHECK(v == Vec2D(-3, -4));
}

TEST_CASE("Vec2D: difference", "[Vec2D]")
{
  Vec2D v(3, 4);
  v = v - Vec2D(3, 5);
  CHECK(v == Vec2D(0, -1));
}

TEST_CASE("Vec2D: assignment by difference", "[Vec2D]")
{
  Vec2D v(3, 4);
  v -= Vec2D(3, 5);
  CHECK(v == Vec2D(0, -1));
}

TEST_CASE("Vec2D: sum", "[Vec2D]")
{
  Vec2D v(3, 4);
  v = v + Vec2D(3, 5);
  CHECK(v == Vec2D(6, 9));
}

TEST_CASE("Vec2D: assignment by sum", "[Vec2D]")
{
  Vec2D v(3, 4);
  v += Vec2D(3, 5);
  CHECK(v == Vec2D(6, 9));
}

TEST_CASE("Vec2D: product", "[Vec2D]")
{
  Vec2D v(3, 4);
  CHECK(v * v == 25);
}

TEST_CASE("Vec2D: scalar product", "[Vec2D]")
{
  Vec2D v(3, 4);
  CHECK(v * 2 == Vec2D(6, 8));

}

TEST_CASE("Vec2D: assignment by scalar product", "[Vec2D]")
{
  Vec2D v(3, 4);
  v *= 2;
  CHECK(v == Vec2D(6, 8));
}

TEST_CASE("Vec2D: scalar division", "[Vec2D]")
{
  Vec2D v(6, 8);
  CHECK(v / 2 == Vec2D(3, 4));
}

TEST_CASE("Vec2D: assignment by scalar division", "[Vec2D]")
{
  Vec2D v(6, 8);
  v /= 2;
  CHECK(v == Vec2D(3, 4));
}

TEST_CASE("Vec2D: operator bool", "[Vec2D]")
{
  Vec2D v;
  CHECK(!v);
  v = Vec2D(3, 0);
  CHECK(v);
  v = Vec2D(0, 4);
  CHECK(v);
  v = Vec2D(3, 4);
  CHECK(v);
}

TEST_CASE("Vec2D: operator less", "[Vec2D]")
{
  CHECK(Vec2D{10, 10} < Vec2D{20, 0});
  CHECK(Vec2D{10, 10} < Vec2D{10, 20});
  CHECK_FALSE(Vec2D{10, 10} < Vec2D{10, 10});
  CHECK_FALSE(Vec2D{20, 10} < Vec2D{10, 10});
}

TEST_CASE("Vec2D: operator greater", "[Vec2D]")
{
  CHECK(Vec2D{20, 0} > Vec2D{10, 10});
  CHECK(Vec2D{10, 20} > Vec2D{10, 10});
  CHECK_FALSE(Vec2D{10, 10} > Vec2D{10, 10});
  CHECK_FALSE(Vec2D{10, 10} > Vec2D{20, 10});
}

TEST_CASE("Vec2D: operator equal", "[Vec2D]")
{
  CHECK(Vec2D{10, 20} == Vec2D{10, 20});
  CHECK_FALSE(Vec2D{10, 20} == Vec2D{10, 30});
  CHECK_FALSE(Vec2D{10, 20} == Vec2D{20, 20});
}
