// file   : src/entity/Avatar.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Avatar.hpp"
#include "Bullet.hpp"
#include "Food.hpp"
#include "Mother.hpp"
#include "Phage.hpp"
#include "Virus.hpp"

#include "../Config.hpp"
#include "../Player.hpp"
#include "../geometry/geometry.hpp"
#include "../serialization.hpp"

Avatar::Avatar(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : Cell(executor, entityFactory, config, id)
{
  type = typeAvatar;
}

void Avatar::setMass(float value)
{
  Cell::setMass(value >= m_config.cellMinMass ? value : m_config.cellMinMass);
  m_maxVelocity = m_config.avatar.maxVelocity - m_config.avatarVelocityDiff *
    (radius - m_config.cellMinRadius) / (m_config.cellRadiusDiff);
}

void Avatar::format(Buffer& buffer)
{
  auto moving = static_cast<bool>(velocity);
  serialize(buffer, static_cast<uint8_t>(type | isNew * newly | isMoving * moving));
  serialize(buffer, id);
  serialize(buffer, position.x);
  serialize(buffer, position.y);
  serialize(buffer, static_cast<uint32_t>(mass));
  serialize(buffer, static_cast<uint16_t>(radius));
  serialize(buffer, color);
  serialize(buffer, player->getId());
  if (moving) {
    serialize(buffer, velocity.x);
    serialize(buffer, velocity.y);
  }
}

void Avatar::interact(Cell& cell)
{
  cell.interact(*this);
}

void Avatar::interact(Avatar& other)
{
  Avatar* attacker = this;
  Avatar* defender = &other;
  if (attacker->mass < defender->mass) {
    std::swap(attacker, defender);
  }
  auto dd = geometry::squareDistance(attacker->position, defender->position);
  auto d = attacker->radius - 0.6 * defender->radius;
  if (attacker->player == defender->player) {
    if (!attacker->isRecombined() || !defender->isRecombined()) {
      return;
    }
    auto d2 = 0.25 * attacker->radius;
    if ((dd > d * d) && (dd > d2 * d2)) {
      return;
    }
    attacker->startRecombination();
  } else {
    if (attacker->mass < 1.25 * defender->mass || dd > d * d) {
      return;
    }
    attacker->player->wakeUp();
  }
  attacker->modifyMass(defender->mass);
  defender->kill();
  defender->player->setKiller(attacker->player->shared_from_this());
}

void Avatar::interact(Food& food)
{
  auto distance = radius + food.radius;
  if (geometry::squareDistance(position, food.position) < distance * distance) {
    player->wakeUp();
    modifyMass(food.mass);
    food.kill();
  }
}

void Avatar::interact(Bullet& bullet)
{
  if (mass < 1.25 * bullet.mass || player == bullet.player && bullet.velocity) {
    return;
  }
  auto d = radius - 0.6 * bullet.radius;
  if (geometry::squareDistance(position, bullet.position) < d * d) {
    if (player != bullet.player) {
      player->wakeUp();
    }
    modifyMass(bullet.mass);
    bullet.kill();
  }
}

void Avatar::interact(Virus& virus)
{
  if (mass < 1.25 * virus.mass) {
    return;
  }
  auto distance = radius - 0.6 * virus.radius;
  if (geometry::squareDistance(position, virus.position) < distance * distance) {
    explode();
    virus.kill();
  }
}

void Avatar::interact(Phage& phage)
{
  if (mass <= m_config.cellMinMass || mass < phage.mass) {
    return;
  }
  auto distance = radius + phage.radius;
  if (geometry::squareDistance(position, phage.position) < distance * distance) {
    auto m = std::min(static_cast<float>(m_config.simulationInterval * phage.mass), mass - m_config.cellMinMass);
    modifyMass(-m);
  }
}

void Avatar::interact(Mother& mother)
{
  auto dist = geometry::distance(position, mother.position);
  if (mother.mass >= 1.25 * mass && dist < mother.radius - 0.25 * radius) {
    mother.modifyMass(mass);
    kill();
  } else if (mass > 1.25 * mother.mass && dist < radius - 0.25 * mother.radius) {
    explode();
    mother.kill();
  }
}

bool Avatar::isAttractiveFor(const Avatar& avatar)
{
  return player != avatar.player && !zombie && mass * 1.25 < avatar.mass;
}

float Avatar::getMaxVelocity() const
{
  return m_maxVelocity;
}

bool Avatar::isRecombined() const
{
  if (!m_recombined) {
    m_recombined = m_fusionTime < TimePoint::clock::now();
  }
  return m_recombined;
}

void Avatar::applyPointAttractionForce(const Vec2D& point, float forceRatio)
{
  Vec2D direction(point - position);
  float distance = direction.length();
  float k = distance < radius ? distance / radius : 1;
  Vec2D v = direction.direction() * (k * m_maxVelocity);
  force += (v - velocity) * (mass * forceRatio);
}

void Avatar::eject(const Vec2D& point)
{
  if (zombie) {
    return;
  }

  float massLoss = m_config.avatar.ejectionMassLoss;
  if (mass < m_config.avatar.ejectionMinMass || mass - massLoss < m_config.cellMinMass) {
    return;
  }

  modifyMass(-massLoss);
  auto& obj = m_entityFactory.createBullet();
  obj.player = player;
  obj.color = color;
  obj.position = position;
  obj.velocity = velocity;
  obj.modifyMass(m_config.avatar.ejectionMass);
  auto direction = point - position;
  if (direction) {
    direction.normalize();
    obj.modifyVelocity(direction * m_config.avatar.ejectionVelocity);
    obj.position += direction * radius;
  }
}

void Avatar::startRecombination()
{
  m_fusionTime = TimePoint::clock::now() + m_config.avatar.recombinationDuration;
  m_recombined = false;
}

void Avatar::setupDeflation()
{
  m_deflationMass = mass * m_config.player.deflationRatio;
}

void Avatar::deflate()
{
  modifyMass(-m_deflationMass);
}

void Avatar::annihilate()
{
  auto& cell = m_entityFactory.createVirus();
  cell.modifyMass(mass);
  cell.position = position;
  cell.color = color;
  kill();
}

void Avatar::explode()
{
  if (m_explosionCallback) {
    m_explosionCallback();
  }
}

void Avatar::setExplosionCallback(const ExplosionCallback& callback)
{
  m_explosionCallback = callback;
}
