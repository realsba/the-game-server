// file   : entity/Mother.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mother.hpp"

#include "src/Room.hpp"

Mother::Mother(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typeMother;
  startRadius = static_cast<float>(config.cellRadiusRatio * sqrt(config.motherStartMass / M_PI));
  color = config.motherColor;
}

void Mother::interact(Cell& cell)
{
  cell.interact(*this);
}

void Mother::interact(Avatar& avatar)
{
  room.interact(avatar, *this);
}

void Mother::interact(Food& food)
{
  room.interact(*this, food);
}

void Mother::interact(Mass& mass)
{
  room.interact(*this, mass);
}

void Mother::interact(Virus& virus)
{
  room.interact(*this, virus);
}

void Mother::interact(Phage& phage)
{
  room.interact(*this, phage);
}

void Mother::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}
