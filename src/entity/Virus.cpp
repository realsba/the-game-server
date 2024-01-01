// file   : entity/Virus.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Virus.hpp"

#include "src/Room.hpp"

Virus::Virus(Room& room)
  : Cell(room)
{
  type = typeVirus;
}

void Virus::interact(Cell& cell)
{
  cell.interact(*this);
}

void Virus::interact(Avatar& avatar)
{
  room.interact(avatar, *this);
}

void Virus::interact(Food& food)
{
  room.interact(*this, food);
}

void Virus::interact(Mass& mass)
{
  room.interact(*this, mass);
}

void Virus::interact(Virus& virus)
{
  room.interact(virus, *this);
}

void Virus::interact(Phage& phage)
{
  room.interact(*this, phage);
}

void Virus::interact(Mother& mother)
{
  room.interact(mother, *this);
}

void Virus::magnetism(Avatar& avatar)
{
  room.magnetism(avatar, *this);
}
