// file   : geometry/Vec2D.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Vec2D.hpp"

#include <ostream>
#include <cmath>

Vec2D::Vec2D(float x, float y) : x(x), y(y) { }

Vec2D Vec2D::direction() const
{
  auto temp(*this);
  temp.normalize();
  return temp;
}

float Vec2D::length() const
{
  return std::sqrt(x * x + y * y);
}

float Vec2D::squareLength() const
{
  return x * x + y * y;
}

void Vec2D::normalize()
{
  auto l = length();
  if (l > 0) {
    x /= l;
    y /= l;
  }
}

void Vec2D::zero()
{
  x = 0;
  y = 0;
}

bool Vec2D::operator==(const Vec2D& other) const
{
  return x == other.x && y == other.y;
}

Vec2D Vec2D::operator-() const
{
  return {-x, -y};
}

Vec2D Vec2D::operator-(const Vec2D& other) const
{
  return {x - other.x, y - other.y};
}

Vec2D& Vec2D::operator-=(const Vec2D& other)
{
  x -= other.x;
  y -= other.y;
  return *this;
}

Vec2D Vec2D::operator+(const Vec2D& other) const
{
  return {x + other.x, y + other.y};
}

Vec2D& Vec2D::operator+=(const Vec2D& other)
{
  x += other.x;
  y += other.y;
  return *this;
}

float Vec2D::operator*(const Vec2D& other) const
{
  return x * other.x + y * other.y;
}

Vec2D Vec2D::operator*(float k) const
{
  return {x * k, y * k};
}

Vec2D& Vec2D::operator*=(float k)
{
  x *= k;
  y *= k;
  return *this;
}

Vec2D Vec2D::operator/(float k) const
{
  return {x / k, y / k};
}

Vec2D& Vec2D::operator/=(float k)
{
  x /= k;
  y /= k;
  return *this;
}

Vec2D::operator bool() const
{
  return x != 0 || y != 0;
}

bool operator<(const Vec2D& a, const Vec2D& b)
{
  return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
}

bool operator>(const Vec2D& a, const Vec2D& b)
{
  return (a.x > b.x) || ((a.x == b.x) && (a.y > b.y));
}

std::ostream& operator<<(std::ostream& os, const Vec2D& obj)
{
  os << "(" << obj.x << ", " << obj.y << ")";
  return os;
}
