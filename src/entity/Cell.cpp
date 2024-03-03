// file   : src/entity/Cell.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Cell.hpp"

#include "src/geometry/geometry.hpp"
#include "src/geometry/AABB.hpp"
#include "src/Config.hpp"

#include "src/serialization.hpp"

Cell::Cell(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : id(id)
  , m_config(config)
  , m_entityFactory(entityFactory)
  , m_deathEvent(executor)
  , m_massChangeEvent(executor)
  , m_motionStartedEvent(executor)
  , m_motionStoppedEvent(executor)
{
  resistanceRatio = config.resistanceRatio;
}

AABB Cell::getAABB() const
{
  Vec2D delta(radius, radius);
  return {position - delta, position + delta};
}

void Cell::modifyMass(float value)
{
  auto old = mass;
  mass += value;
  if (mass < MIN_MASS) {
    mass = MIN_MASS;
  }
  if (!materialPoint) {
    radius = m_config.cellRadiusRatio * sqrt(mass / M_PI);
  }
  m_massChangeEvent.notify(mass - old);
}

void Cell::modifyVelocity(const Vec2D& value)
{
  velocity += value;
  m_motionStartedEvent.notify();
}

void Cell::applyResistanceForce()
{
  force -= velocity.direction() * (radius * resistanceRatio);
}

bool Cell::intersects(const AABB& box)
{
  return geometry::intersects(box, *this);
}

void Cell::simulate(double dt)
{
  auto acceleration = force / mass;
  auto prevVelocity = velocity;
  velocity += acceleration * dt;
  if (std::fabs(1 + velocity.direction() * prevVelocity.direction()) <= 0.01) {
    velocity.zero();
    m_motionStoppedEvent.notify();
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

bool Cell::isAttractiveFor(const Avatar& avatar)
{
  return false;
}

void Cell::kill()
{
  zombie = true;
  m_deathEvent.notify();
}

void Cell::startMotion()
{
  m_motionStartedEvent.notify();
}

void Cell::subscribeToDeathEvent(void* tag, Event<>::Handler&& handler)
{
  m_deathEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromDeathEvent(void* tag)
{
  m_deathEvent.unsubscribe(tag);
}

void Cell::subscribeToMassChangeEvent(void* tag, Event<float>::Handler&& handler)
{
  m_massChangeEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMassChangeEvent(void* tag)
{
  m_massChangeEvent.unsubscribe(tag);
}

void Cell::subscribeToMotionStartedEvent(void* tag, Event<>::Handler&& handler)
{
  m_motionStartedEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMotionStartedEvent(void* tag)
{
  m_motionStartedEvent.unsubscribe(tag);
}

void Cell::subscribeToMotionStoppedEvent(void *tag, Event<>::Handler &&handler)
{
  m_motionStoppedEvent.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMotionStoppedEvent(void *tag)
{
  m_motionStoppedEvent.unsubscribe(tag);
}

