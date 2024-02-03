// file   : src/entity/Mass.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mass.hpp"

#include "src/Room.hpp"

Mass::Mass(Room& room, uint32_t id)
  : Cell(room, id)
{
  type = typeMass;
}

void Mass::interact(Cell& cell)
{
  cell.interact(*this);
}

void Mass::interact(Avatar& avatar)
{
  room.interact(avatar, *this);
}

void Mass::interact(Virus& virus)
{
  room.interact(virus, *this);
}

void Mass::interact(Phage& phage)
{
  room.interact(phage, *this);
}

void Mass::interact(Mother& mother)
{
  room.interact(mother, *this);
}

void Mass::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}

bool Mass::isAttractiveFor(const Avatar& avatar)
{
  return true;
}
