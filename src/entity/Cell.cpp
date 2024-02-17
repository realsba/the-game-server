// file   : src/entity/Cell.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Cell.hpp"

#include "src/geometry/geometry.hpp"
#include "src/packet/serialization.hpp"

#include "src/Player.hpp"
#include "src/Room.hpp"

#include <spdlog/spdlog.h>

Cell::Cell(Room& room, uint32_t id)
  : room(room)
  , m_config(room.getConfig())
  , id(id)
  , m_deathEvent(room.getExecutor())
  , m_massChangeEvent(room.getExecutor())
  , m_motionStartedEvent(room.getExecutor())
{
  const auto& config = room.getConfig();
  resistanceRatio = config.resistanceRatio;
}

AABB Cell::getAABB() const
{
  Vec2D delta(radius, radius);
  return {position - delta, position + delta};
}

void Cell::modifyMass(float value)
{
  const auto& config = room.getConfig();
  auto old = mass;
  mass += value;
  if (mass < config.cellMinMass) {
    mass = config.cellMinMass;
  }
  radius = config.cellRadiusRatio * sqrt(mass / M_PI);
  m_massChangeEvent.notify(this, mass - old);
}

void Cell::modifyVelocity(const Vec2D& value)
{
  velocity += value;
  m_motionStartedEvent.notify(this);
}

void Cell::applyImpulse(const Vec2D& value)
{
  velocity += value / mass;
}

void Cell::applyResistanceForce()
{
  force -= velocity.direction() * (radius * resistanceRatio);
}

bool Cell::shouldBeProcessed() const
{
  return static_cast<bool>(velocity);
}

bool Cell::intersects(const AABB& box)
{
  return geometry::intersects(box, *this);
}

void Cell::simulate(float dt)
{
  auto acceleration = force / mass;
  auto prevVelocity = velocity;
  velocity += acceleration * dt;
  if (std::fabs(1 + velocity.direction() * prevVelocity.direction()) <= 0.01) {
    velocity.zero();
  } else {
    position += velocity * dt;
  }
  force.zero();
}

void Cell::format(Buffer& buffer)
{
  auto moving = static_cast<bool>(velocity);
  serialize(buffer, static_cast<uint8_t>(type | isNew * newly | isMoving * moving));
  serialize(buffer, id);
  serialize(buffer, position.x);
  serialize(buffer, position.y);
  serialize(buffer, static_cast<uint32_t>(mass));
  serialize(buffer, static_cast<uint16_t>(radius));
  serialize(buffer, color);
  if (moving) {
    serialize(buffer, velocity.x);
    serialize(buffer, velocity.y);
  }
}

void Cell::interact(Cell&) { }

void Cell::interact(Avatar&) { }

void Cell::interact(Food&) { }

void Cell::interact(Bullet&) { }

void Cell::interact(Virus&) { }

void Cell::interact(Phage&) { }

void Cell::interact(Mother&) { }

void Cell::attract(Avatar& avatar) { }

bool Cell::isAttractiveFor(const Avatar& avatar)
{
  return false;
}

void Cell::kill()
{
  zombie = true;
  m_deathEvent.notify(this);
}

void Cell::startMotion()
{
  m_motionStartedEvent.notify(this);
}

void Cell::subscribeToDeathEvent(void* tag, Event<Cell*>::Handler&& handler)
{
  m_deathEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromDeathEvent(void* tag)
{
  m_deathEvent.unsubscribe(tag);
}

void Cell::subscribeToMassChangeEvent(void* tag, Event<Cell*, float>::Handler&& handler)
{
  m_massChangeEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMassChangeEvent(void* tag)
{
  m_massChangeEvent.unsubscribe(tag);
}

void Cell::subscribeToMotionStartedEvent(void* tag, Event<Cell*>::Handler&& handler)
{
  m_motionStartedEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMotionStartedEvent(void* tag)
{
  m_motionStartedEvent.unsubscribe(tag);
}

