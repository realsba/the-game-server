// file   : entity/Cell.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef ENTITY_CELL_HPP
#define ENTITY_CELL_HPP

#include "src/geometry/Circle.hpp"

#include <cstdint>
#include <string>
#include <set>

class MemoryStream;
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
  Cell(Room& room);
  virtual ~Cell();

  AABB getAABB() const;

  void applayImpulse(const Vec2D& value);

  virtual bool intersects(const AABB& box);
  virtual void simulate(float dt);
  virtual void format(MemoryStream& ms);

  virtual void interact(Cell& cell);
  virtual void interact(Avatar& avatar);
  virtual void interact(Food& food);
  virtual void interact(Mass& mass);
  virtual void interact(Virus& virus);
  virtual void interact(Phage& phage);
  virtual void interact(Mother& mother);

  virtual void magnetism(Avatar& avatar);
  virtual bool isAttractive(const Avatar& avatar);

  enum Type {
    typeAvatar = 1,
    typeFood = 2,
    typeMass = 3,
    typeVirus = 4,
    typePhage = 5,
    typeMother = 6,

    typeNew = 64,
    typeMoving = 128
  };

  std::set<Sector*> sectors;
  Sector* leftTopSector {nullptr};
  Sector* rightBottomSector {nullptr};

  Vec2D     velocity;
  Vec2D     force;
  Room&     room;
  Cell*     creator {nullptr};
  Player*   player {nullptr};
  float     mass {0};
  float     resistanceRatio {0};
  uint32_t  id {0};
  uint32_t  type {0};
  uint8_t   color {0};
  bool      newly {true};
  bool      zombie {false};
  bool      materialPoint {false};
};

#endif /* ENTITY_CELL_HPP */
