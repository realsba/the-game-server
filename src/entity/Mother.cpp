// file   : src/entity/Mother.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mother.hpp"

#include "Avatar.hpp"
#include "Food.hpp"
#include "Bullet.hpp"
#include "Virus.hpp"
#include "Phage.hpp"

#include "src/geometry/geometry.hpp"
#include "src/Room.hpp"

Mother::Mother(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typeMother;
  color = config.mother.color;
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

void Mother::interact(Bullet& bullet)
{
  auto distance = radius - 0.25 * bullet.radius;
  if (geometry::squareDistance(position, bullet.position) < distance * distance) {
    modifyMass(bullet.mass);
    bullet.kill();
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
