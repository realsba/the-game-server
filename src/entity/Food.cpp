// file   : src/entity/Food.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Avatar.hpp"
#include "Bullet.hpp"
#include "Food.hpp"
#include "Mother.hpp"
#include "Phage.hpp"
#include "Virus.hpp"

#include "../Config.hpp"
#include "../geometry/geometry.hpp"

Food::Food(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : Cell(executor, entityFactory, config, id)
{
  type = typeFood;
  radius = config.food.radius;
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
  return !zombie;
}
