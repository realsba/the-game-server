// file   : src/entity/Mother.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mother.hpp"

#include "Food.hpp"
#include "Mass.hpp"
#include "Virus.hpp"
#include "Phage.hpp"

#include "src/geometry/geometry.hpp"
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
  avatar.interact(*this);
}

void Mother::interact(Food& food)
{
  if (food.creator == this && food.velocity) {
    return;
  }
  if (geometry::squareDistance(position, food.position) < radius * radius) {
    food.kill();
  }
}

void Mother::interact(Mass& target)
{
  auto distance = radius - 0.25 * target.radius;
  if (geometry::squareDistance(position, target.position) < distance * distance) {
    modifyMass(target.mass);
    target.kill();
  }
}

void Mother::interact(Virus& virus)
{
  auto distance = radius + virus.radius;
  if (geometry::squareDistance(position, virus.position) < distance * distance) {
    modifyMass(virus.mass);
    virus.kill();
  }
}

void Mother::interact(Phage& phage)
{
  auto distance = radius + phage.radius;
  if (geometry::squareDistance(position, phage.position) < distance * distance) {
    modifyMass(phage.mass);
    phage.kill();
  }
}

void Mother::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}
