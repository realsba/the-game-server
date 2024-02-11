// file   : src/entity/Virus.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Virus.hpp"

#include "Avatar.hpp"
#include "Food.hpp"
#include "Bullet.hpp"
#include "Phage.hpp"
#include "Mother.hpp"

#include "src/geometry/geometry.hpp"
#include "src/Room.hpp"

Virus::Virus(Room& room, uint32_t id)
  : Cell(room, id)
{
  const auto& config = room.getConfig();
  type = typeVirus;
  color = config.virusColor;
}

void Virus::interact(Cell& cell)
{
  cell.interact(*this);
}

void Virus::interact(Avatar& avatar)
{
  avatar.interact(*this);
}

void Virus::interact(Food& food)
{
  auto distance = radius + food.radius;
  if (geometry::squareDistance(position, food.position) < distance * distance) {
    food.kill();
  }
}

void Virus::interact(Bullet& bullet)
{
  auto distance = radius + bullet.radius;
  if (geometry::squareDistance(position, bullet.position) < distance * distance) {
    auto relativeVelocity = velocity - bullet.velocity;
    auto normal = (position - bullet.position).direction();
    auto impulse = relativeVelocity * normal * (2.0f * mass * bullet.mass / (mass + bullet.mass));
    velocity -= normal * impulse / mass;
    startMotion();
    bullet.kill();
  }
}

void Virus::interact(Virus& other)
{
  auto distance = radius + other.radius;
  if (geometry::squareDistance(position, other.position) < distance * distance) {
    auto& obj = room.createVirus();
    obj.modifyMass(mass + other.mass);
    obj.position = (position + other.position) * 0.5;
    kill();
    other.kill();
  }
}

void Virus::interact(Phage& phage)
{
  auto distance = radius + phage.radius;
  if (geometry::squareDistance(position, phage.position) < distance * distance) {
    auto& obj = room.createMother();
    obj.modifyMass(mass + phage.mass);
    obj.position = (position + phage.position) * 0.5;
    kill();
    phage.kill();
  }
}

void Virus::interact(Mother& mother)
{
  mother.interact(*this);
}

void Virus::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}
