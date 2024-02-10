// file   : src/entity/Mass.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mass.hpp"

#include "Avatar.hpp"
#include "Virus.hpp"
#include "Phage.hpp"
#include "Mother.hpp"

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
  avatar.interact(*this);
}

void Mass::interact(Virus& virus)
{
  virus.interact(*this);
}

void Mass::interact(Phage& phage)
{
  phage.interact(*this);
}

void Mass::interact(Mother& mother)
{
  mother.interact(*this);
}

void Mass::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}

bool Mass::isAttractiveFor(const Avatar& avatar)
{
  return true;
}
