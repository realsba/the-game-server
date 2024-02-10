// file   : src/entity/Cell.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_CELL_HPP
#define THEGAME_ENTITY_CELL_HPP

#include "src/geometry/Circle.hpp"
#include "src/TimePoint.hpp"
#include "src/types.hpp"

#include <cstdint>
#include <string>

class Sector;
class Player;
class Room;
class AABB;

class Avatar;
class Food;
class Mass;
class Virus;
class Phage;
class Mother;

class Cell : public Circle {
public:
  explicit Cell(Room& room, uint32_t id = 0);

  virtual ~Cell() = default;

  [[nodiscard]] AABB getAABB() const;

  void applyVelocity(const Vec2D& value);
  void applyImpulse(const Vec2D& value);
  void applyResistanceForce();

  virtual bool shouldBeProcessed() const;
  virtual bool intersects(const AABB& box);
  virtual void simulate(float dt);
  virtual void format(Buffer& buffer);

  virtual void interact(Cell&);
  virtual void interact(Avatar&);
  virtual void interact(Food&);
  virtual void interact(Mass&);
  virtual void interact(Virus&);
  virtual void interact(Phage&);
  virtual void interact(Mother&);

  virtual void attract(Avatar& avatar);
  virtual bool isAttractiveFor(const Avatar& avatar);

  enum Type {
    typeAvatar = 1,
    typeFood = 2,
    typeMass = 3,
    typeVirus = 4,
    typePhage = 5,
    typeMother = 6,

    isNew = 64,
    isMoving = 128
  };

  std::set<Sector*> sectors;
  Sector*           leftTopSector {nullptr};
  Sector*           rightBottomSector {nullptr};
  TimePoint         created {TimePoint::clock::now()};
  Vec2D             velocity;
  Vec2D             force;
  Room&             room;
  Cell*             creator {nullptr};
  Player*           player {nullptr};
  float             mass {0};
  float             resistanceRatio {0};
  uint32_t          id {0};
  uint32_t          type {0};
  uint8_t           color {0};
  bool              newly {true};
  bool              zombie {false};
  bool              materialPoint {false};
};

#endif /* THEGAME_ENTITY_CELL_HPP */
