// file   : entity/Cell.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Cell.hpp"

#include "src/geometry/geometry.hpp"
#include "src/MemoryStream.hpp"
#include "src/Player.hpp"
#include "src/Room.hpp"

Cell::Cell(Room& room) :
  room(room)
{
  const auto& config = room.getConfig();
  resistanceRatio = config.resistanceRatio;
}

AABB Cell::getAABB() const
{
  Vec2D delta(radius, radius);
  return {position - delta, position + delta};
}

void Cell::applyImpulse(const Vec2D& value)
{
  velocity += value / mass;
}

bool Cell::intersects(const AABB& box)
{
  return geometry::intersects(box, *this);
}

void Cell::simulate(float dt)
{
  Vec2D resistanceForce = velocity.direction() * (radius * resistanceRatio);
  force -= resistanceForce;
  Vec2D acceleration = force / mass;
  Vec2D prev = velocity;
  velocity += acceleration * dt;
  if (std::fabs(1 + velocity.direction() * prev.direction()) <= 0.01) {
    velocity.zero();
  } else {
    position += velocity * dt;
  }
  force.zero();
}

void Cell::format(MemoryStream& ms)
{
  bool moving = static_cast<bool>(velocity);
  ms.writeUInt8(type | typeNew * newly | typeMoving * moving);
  ms.writeUInt32(id);
  ms.writeFloat(position.x);
  ms.writeFloat(position.y);
  ms.writeUInt32(static_cast<uint32_t>(mass));
  ms.writeUInt16(static_cast<uint16_t>(radius));
  ms.writeUInt8(color);
  if (moving) {
    ms.writeFloat(velocity.x);
    ms.writeFloat(velocity.y);
  }
}

void Cell::interact(Cell&) { }

void Cell::interact(Avatar&) { }

void Cell::interact(Food&) { }

void Cell::interact(Mass&) { }

void Cell::interact(Virus&) { }

void Cell::interact(Phage&) { }

void Cell::interact(Mother&) { }

void Cell::magnetism(Avatar& avatar) { }

bool Cell::isAttractive(const Avatar& avatar)
{
  return false;
}
