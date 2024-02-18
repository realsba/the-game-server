// file   : src/entity/Food.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Food.hpp"

#include "Bullet.hpp"
#include "Virus.hpp"
#include "Phage.hpp"
#include "Mother.hpp"

#include "src/geometry/geometry.hpp"
#include "src/Room.hpp"

Food::Food(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typeFood;
  materialPoint = true;
  resistanceRatio = config.food.resistanceRatio;
}

bool Food::intersects(const AABB& box)
{
  return geometry::intersects(box, position);
}

void Food::interact(Cell& cell)
{
  cell.interact(*this);
}

void Food::interact(Avatar& avatar)
{
  avatar.interact(*this);
}

void Food::interact(Virus& virus)
{
  virus.interact(*this);
}

void Food::interact(Phage& phage)
{
  phage.interact(*this);
}

void Food::interact(Mother& mother)
{
  mother.interact(*this);
}

bool Food::isAttractiveFor(const Avatar& avatar)
{
  return true;
}
