// file   : src/entity/Phage.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Phage.hpp"

#include "Avatar.hpp"
#include "Food.hpp"
#include "Bullet.hpp"
#include "Virus.hpp"
#include "Mother.hpp"

#include "src/geometry/geometry.hpp"
#include "src/Config.hpp"

Phage::Phage(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : Cell(executor, entityFactory, config, id)
{
  type = typePhage;
  color = config.phage.color;
}

void Phage::interact(Cell& cell)
{
  cell.interact(*this);
}

void Phage::interact(Avatar& avatar)
{
  avatar.interact(*this);
}

void Phage::interact(Food& food)
{
  auto distance = radius + food.radius;
  if (geometry::squareDistance(position, food.position) < distance * distance) {
    food.kill();
  }
}

void Phage::interact(Bullet& target)
{
  auto distance = radius + target.radius;
  if (geometry::squareDistance(position, target.position) < distance * distance) {
    auto relativeVelocity = velocity - target.velocity;
    auto normal = (position - target.position).direction();
    auto impulse = relativeVelocity * normal * (2.0f * mass * target.mass / (mass + target.mass));
    velocity -= normal * impulse / mass;
    startMotion();
    target.kill();
  }
}

void Phage::interact(Virus& virus)
{
  virus.interact(*this);
}

void Phage::interact(Phage& other)
{
  auto distance = radius + other.radius;
  if (geometry::squareDistance(position, other.position) < distance * distance) {
    auto& obj = m_entityFactory.createPhage();
    obj.modifyMass(mass + other.mass);
    obj.position = (position + other.position) * 0.5;
    kill();
    other.kill();
  }
}

void Phage::interact(Mother& mother)
{
  mother.interact(*this);
}
