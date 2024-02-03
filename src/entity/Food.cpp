// file   : src/entity/Food.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Food.hpp"

#include "src/geometry/geometry.hpp"
#include "src/Room.hpp"

Food::Food(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typeFood;
  materialPoint = true;
  resistanceRatio = config.foodResistanceRatio;
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
  room.interact(avatar, *this);
}

void Food::interact(Virus& virus)
{
  room.interact(virus, *this);
}

void Food::interact(Phage& phage)
{
  room.interact(phage, *this);
}

void Food::interact(Mother& mother)
{
  room.interact(mother, *this);
}

void Food::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}
