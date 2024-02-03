// file   : src/geometry/Vec2D.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_GEOMETRY_VEC2D_HPP
#define THEGAME_GEOMETRY_VEC2D_HPP

class Vec2D {
public:
  Vec2D() = default;
  Vec2D(float x, float y);

  [[nodiscard]] Vec2D direction() const;
  [[nodiscard]] float length() const;
  [[nodiscard]] float squareLength() const;
  void normalize();
  void zero();

  bool operator==(const Vec2D& other) const;

  Vec2D operator-() const;

  Vec2D operator-(const Vec2D& other) const;
  Vec2D& operator-=(const Vec2D& other);
  Vec2D operator+(const Vec2D& other) const;
  Vec2D& operator+=(const Vec2D& other);
  float operator*(const Vec2D& other) const;

  Vec2D operator*(float k) const;
  Vec2D& operator*=(float k);
  Vec2D operator/(float k) const;
  Vec2D& operator/=(float k);

  explicit operator bool() const;

  float x {0};
  float y {0};
};

bool operator<(const Vec2D& a, const Vec2D& b);
bool operator>(const Vec2D& a, const Vec2D& b);

#endif /* THEGAME_GEOMETRY_VEC2D_HPP */
