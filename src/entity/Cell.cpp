// file   : src/entity/Cell.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Cell.hpp"

#include "../Config.hpp"
#include "../geometry/AABB.hpp"
#include "../geometry/geometry.hpp"
#include "../serialization.hpp"

Cell::Cell(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : id(id)
  , m_config(config)
  , m_entityFactory(entityFactory)
  , m_deathEmitter(entityFactory.getDeathExecutor())
  , m_massChangedEmitter(entityFactory.getGameExecutor())
  , m_motionStartedEmitter(entityFactory.getGameExecutor())
  , m_motionStoppedEmitter(entityFactory.getGameExecutor())
{
  resistanceRatio = config.resistanceRatio;
}

AABB Cell::getAABB() const
{
  Vec2D delta(radius, radius);
  return {position - delta, position + delta};
}

void Cell::setMass(float value)
{
  auto previousMass = mass;
  mass = value >= MIN_MASS ? value : MIN_MASS;
  if (!materialPoint) {
    radius = m_config.cellRadiusRatio * sqrt(mass / M_PI);
  }
  m_massChangedEmitter.emit(mass - previousMass);
}

void Cell::modifyMass(float value)
{
  setMass(mass + value);
}

void Cell::modifyVelocity(const Vec2D& value)
{
  velocity += value;
  m_motionStartedEmitter.emit();
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
    m_motionStoppedEmitter.emit();
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
  m_deathEmitter.emit();
}

void Cell::startMotion()
{
  m_motionStartedEmitter.emit();
}

void Cell::subscribeToDeath(void* tag, EventEmitter<>::Handler&& handler)
{
  m_deathEmitter.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromDeath(void* tag)
{
  m_deathEmitter.unsubscribe(tag);
}

void Cell::subscribeToMassChange(void* tag, EventEmitter<float>::Handler&& handler)
{
  m_massChangedEmitter.subscribe(tag, std::move(handler));
}

void Cell::unsubscribeFromMassChange(void* tag)
{
  m_massChangedEmitter.unsubscribe(tag);
}

void Cell::subscribeToMotionStarted(void* tag, EventEmitter<>::Handler&& handler)
{
  m_motionStartedEmitter.subscribe(tag, std::move(handler));
}

void Cell::subscribeToMotionStopped(void* tag, EventEmitter<>::Handler&& handler)
{
  m_motionStoppedEmitter.subscribe(tag, std::move(handler));
}
