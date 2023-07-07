/*
 * File:   test_Vec2D.cpp
 * Author: sba
 */

#include "test_Vec2D.hpp"

#include "src/geometry/Vec2D.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(test_Vec2D);

test_Vec2D::test_Vec2D()
{
}

test_Vec2D::~test_Vec2D()
{
}

void test_Vec2D::setUp()
{
}

void test_Vec2D::tearDown()
{
}

void test_Vec2D::test_direction()
{
  Vec2D v;
  CPPUNIT_ASSERT(v.direction() == v);
  CPPUNIT_ASSERT(v.direction() == Vec2D());

  v = Vec2D(100, 0);
  CPPUNIT_ASSERT(v.direction() == Vec2D(1, 0));

  v = Vec2D(-100, 0);
  CPPUNIT_ASSERT(v.direction() == Vec2D(-1, 0));

  v = Vec2D(0, 100);
  CPPUNIT_ASSERT(v.direction() == Vec2D(0, 1));

  v = Vec2D(0, -100);
  CPPUNIT_ASSERT(v.direction() == Vec2D(0, -1));
}

void test_Vec2D::test_length()
{
  Vec2D v(42, 0);
  CPPUNIT_ASSERT(v.length() == 42);
  v = Vec2D(0, 42);
  CPPUNIT_ASSERT(v.length() == 42);
  v = Vec2D(3, 4);
  CPPUNIT_ASSERT(v.length() == 5);
  v = Vec2D(4, 3);
  CPPUNIT_ASSERT(v.length() == 5);
}

void test_Vec2D::test_squareLength()
{
  Vec2D v(42, 0);
  CPPUNIT_ASSERT(v.squareLength() == 42 * 42);
  v = Vec2D(0, 42);
  CPPUNIT_ASSERT(v.squareLength() == 42 * 42);
  v = Vec2D(3, 4);
  CPPUNIT_ASSERT(v.squareLength() == 5 * 5);
  v = Vec2D(4, 3);
  CPPUNIT_ASSERT(v.squareLength() == 5 * 5);
}

void test_Vec2D::test_normalize()
{
  Vec2D v(42, 0);
  v.normalize();
  CPPUNIT_ASSERT(v == Vec2D(1, 0));
  v = Vec2D(0, 42);
  v.normalize();
  CPPUNIT_ASSERT(v == Vec2D(0, 1));
}

void test_Vec2D::test_zero()
{
  Vec2D v(3, 4);
  v.zero();
  CPPUNIT_ASSERT(v == Vec2D(0, 0));
}

void test_Vec2D::test_unaryMinus()
{
  Vec2D v(3, 4);
  v = -v;
  CPPUNIT_ASSERT(v == Vec2D(-3, -4));
}

void test_Vec2D::test_difference()
{
  Vec2D v(3, 4);
  v = v - Vec2D(3, 5);
  CPPUNIT_ASSERT(v == Vec2D(0, -1));
}

void test_Vec2D::test_assignmentByDifference()
{
  Vec2D v(3, 4);
  v -= Vec2D(3, 5);
  CPPUNIT_ASSERT(v == Vec2D(0, -1));
}

void test_Vec2D::test_sum()
{
  Vec2D v(3, 4);
  v = v + Vec2D(3, 5);
  CPPUNIT_ASSERT(v == Vec2D(6, 9));
}

void test_Vec2D::test_assignmentBySum()
{
  Vec2D v(3, 4);
  v += Vec2D(3, 5);
  CPPUNIT_ASSERT(v == Vec2D(6, 9));
}

void test_Vec2D::test_product()
{
  Vec2D v(3, 4);
  CPPUNIT_ASSERT(v * v == 25);
}

void test_Vec2D::test_scalarProduct()
{
  Vec2D v(3, 4);
  CPPUNIT_ASSERT(v * 2 == Vec2D(6, 8));
}

void test_Vec2D::test_assignmentByScalarProduct()
{
  Vec2D v(3, 4);
  v *= 2;
  CPPUNIT_ASSERT(v == Vec2D(6, 8));
}

void test_Vec2D::test_scalarDivision()
{
  Vec2D v(6, 8);
  CPPUNIT_ASSERT(v / 2 == Vec2D(3, 4));
}

void test_Vec2D::test_assignmentByScalarDivision()
{
  Vec2D v(6, 8);
  v /= 2;
  CPPUNIT_ASSERT(v == Vec2D(3, 4));
}

void test_Vec2D::test_operatorBool()
{
  Vec2D v;
  CPPUNIT_ASSERT(!v);
  v = Vec2D(3, 0);
  CPPUNIT_ASSERT(v);
  v = Vec2D(0, 4);
  CPPUNIT_ASSERT(v);
  v = Vec2D(3, 4);
  CPPUNIT_ASSERT(v);
}
