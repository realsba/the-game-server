// file   : entity/Mass.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mass.hpp"

#include "src/Room.hpp"

Mass::Mass(Room& room) :
  Cell(room)
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

void Mass::magnetism(Avatar& avatar)
{
  room.magnetism(avatar, *this);
}

bool Mass::isAttractive(const Avatar& avatar)
{
  return true;
}
