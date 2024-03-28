// file   : tests/geometry/Test_AABB.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include "../../src/geometry/AABB.hpp"
#include "../../src/geometry/formatter.hpp"

TEST_CASE("AABB: Default Constructor", "[AABB]")
{
  AABB aabb;
  CHECK(aabb.a.x == 0);
  CHECK(aabb.a.y == 0);
  CHECK(aabb.b.x == 0);
  CHECK(aabb.b.y == 0);
}

TEST_CASE("AABB: Constructor AABB(float, float, float, float)", "[AABB]")
{
  AABB aabb{1, 2, 3, 4};
  CHECK(aabb.a.x == 1);
  CHECK(aabb.a.y == 2);
  CHECK(aabb.b.x == 3);
  CHECK(aabb.b.y == 4);
}

TEST_CASE("AABB: Constructor AABB(const Vec2D&, const Vec2D&)", "[AABB]")
{
  AABB aabb{{11, 22}, {33, 44}};
  CHECK(aabb.a.x == 11);
  CHECK(aabb.a.y == 22);
  CHECK(aabb.b.x == 33);
  CHECK(aabb.b.y == 44);
}

TEST_CASE("AABB: normalize", "[AABB]")
{
  AABB aabb{{33, 44}, {11, 22}};
  aabb.normalize();
  CHECK(aabb.a.x == 11);
  CHECK(aabb.a.y == 22);
  CHECK(aabb.b.x == 33);
  CHECK(aabb.b.y == 44);

  aabb = {{1, 2}, {3, 4}};
  aabb.normalize();
  CHECK(aabb.a.x == 1);
  CHECK(aabb.a.y == 2);
  CHECK(aabb.b.x == 3);
  CHECK(aabb.b.y == 4);
}

TEST_CASE("AABB: translate", "[AABB]")
{
  AABB aabb{{11, 22}, {33, 44}};
  aabb.translate({100, 200});
  CHECK(aabb.a.x == 111);
  CHECK(aabb.a.y == 222);
  CHECK(aabb.b.x == 133);
  CHECK(aabb.b.y == 244);
}

TEST_CASE("AABB: contain", "[AABB]")
{
  AABB aabb{{10, 20}, {30, 40}};

  CHECK(aabb.contain({25, 35}));
  CHECK(aabb.contain({10, 35}));
  CHECK(aabb.contain({30, 35}));
  CHECK(aabb.contain({25, 20}));
  CHECK(aabb.contain({25, 40}));
  CHECK(aabb.contain({5, 35}) == false);
  CHECK(aabb.contain({35, 35}) == false);
  CHECK(aabb.contain({25, 10}) == false);
  CHECK(aabb.contain({25, 45}) == false);
}

TEST_CASE("AABB: width", "[AABB]")
{
  AABB aabb{{10, 20}, {30, 40}};
  CHECK(aabb.width() == 20);
}

TEST_CASE("AABB: height", "[AABB]")
{
  AABB aabb{{10, 20}, {30, 45}};
  CHECK(aabb.height() == 25);
}

TEST_CASE("AABB: operator<<", "[AABB]")
{
  AABB aabb{{10, 20}, {30, 40}};
  CHECK(fmt::format("{}", aabb) == "[(10, 20); (30, 40)]");
}

