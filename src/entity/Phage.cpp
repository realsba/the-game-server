// file   : entity/Phage.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Phage.hpp"

#include "src/Room.hpp"

Phage::Phage(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typePhage;
  color = config.phageColor;
}

void Phage::interact(Cell& cell)
{
  cell.interact(*this);
}

void Phage::interact(Avatar& avatar)
{
  room.interact(avatar, *this);
}

void Phage::interact(Food& food)
{
  room.interact(*this, food);
}

void Phage::interact(Mass& mass)
{
  room.interact(*this, mass);
}

void Phage::interact(Virus& virus)
{
  room.interact(virus, *this);
}

void Phage::interact(Phage& phage)
{
  room.interact(phage, *this);
}

void Phage::interact(Mother& mother)
{
  room.interact(mother, *this);
}

void Phage::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}
