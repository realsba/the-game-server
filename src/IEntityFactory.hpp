// file   : src/IEntityFactory.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_I_ENTITY_FACTORY_HPP
#define THEGAME_I_ENTITY_FACTORY_HPP

#include <cstdint>

class Avatar;
class Food;
class Bullet;
class Virus;
class Phage;
class Mother;
class Vec2D;

class IEntityFactory {
public:
  virtual ~IEntityFactory() = default;

  virtual Avatar& createAvatar() = 0;
  virtual Food& createFood() = 0;
  virtual Bullet& createBullet() = 0;
  virtual Virus& createVirus() = 0;
  virtual Phage& createPhage() = 0;
  virtual Mother& createMother() = 0;

  [[nodiscard]] virtual Vec2D getRandomPosition(double radius) const = 0;
  [[nodiscard]] virtual Vec2D getRandomDirection() const = 0;
};

#endif /* THEGAME_I_ENTITY_FACTORY_HPP */
