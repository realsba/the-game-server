/*
 * File:   test_Vec2D.hpp
 * Author: sba
 */

#ifndef TEST_VEC2D_HPP
#define TEST_VEC2D_HPP

#include <cppunit/extensions/HelperMacros.h>

class test_Vec2D : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(test_Vec2D);

  CPPUNIT_TEST(test_direction);
  CPPUNIT_TEST(test_length);
  CPPUNIT_TEST(test_squareLength);
  CPPUNIT_TEST(test_normalize);
  CPPUNIT_TEST(test_zero);
  CPPUNIT_TEST(test_unaryMinus);
  CPPUNIT_TEST(test_difference);
  CPPUNIT_TEST(test_assignmentByDifference);
  CPPUNIT_TEST(test_sum);
  CPPUNIT_TEST(test_assignmentBySum);
  CPPUNIT_TEST(test_product);
  CPPUNIT_TEST(test_scalarProduct);
  CPPUNIT_TEST(test_assignmentByScalarProduct);
  CPPUNIT_TEST(test_scalarDivision);
  CPPUNIT_TEST(test_assignmentByScalarDivision);
  CPPUNIT_TEST(test_operatorBool);

  CPPUNIT_TEST_SUITE_END();

public:
  test_Vec2D();
  virtual ~test_Vec2D();
  void setUp();
  void tearDown();

private:
  void test_direction();
  void test_length();
  void test_squareLength();
  void test_normalize();
  void test_zero();
  void test_unaryMinus();
  void test_difference();
  void test_assignmentByDifference();
  void test_sum();
  void test_assignmentBySum();
  void test_product();
  void test_scalarProduct();
  void test_assignmentByScalarProduct();
  void test_scalarDivision();
  void test_assignmentByScalarDivision();
  void test_operatorBool();
};

#endif /* TEST_VEC2D_HPP */
