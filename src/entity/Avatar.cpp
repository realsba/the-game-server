// file   : src/entity/Avatar.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Avatar.hpp"

#include "Food.hpp"
#include "Bullet.hpp"
#include "Virus.hpp"
#include "Phage.hpp"
#include "Mother.hpp"

#include "src/serialization.hpp"
#include "src/geometry/geometry.hpp"
#include "src/Player.hpp"
#include "src/Room.hpp"

Avatar::Avatar(Room& room, uint32_t id)
  : Cell(room, id)
{
  type = typeAvatar;
}

void Avatar::modifyMass(float value)
{
  Cell::modifyMass(value);
  const auto& config = room.getConfig();
  maxVelocity = config.avatar.maxVelocity - config.avatarVelocityDiff * (radius - config.cellMinRadius) / (config.cellRadiusDiff);
}

bool Avatar::shouldBeProcessed() const
{
  return player->getAvatars().size() > 1 || Cell::shouldBeProcessed();
}

void Avatar::simulate(double dt)
{
  Cell::simulate(dt);
  if (m_recombinationTime > 0) {
    m_recombinationTime -= dt;
    if (m_recombinationTime <= 0) {
      m_recombinationTime = 0;
      m_recombined = true;
    }
  }
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
    attacker->recombination(room.getConfig().avatar.recombinationTime);
  } else {
    if (attacker->mass < 1.25 * defender->mass || dd > d * d) {
      return;
    }
    attacker->player->wakeUp();
  }
  attacker->modifyMass(defender->mass);
  defender->kill();
  Player& player = *defender->player;
  player.removeAvatar(defender);
  if (player.isDead()) {
    player.killer = attacker->player;
  }
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
    room.explode(*this);
    virus.kill();
  }
}

void Avatar::interact(Phage& phage)
{
  const auto& config = room.getConfig();
  if (mass <= config.cellMinMass || mass < phage.mass) {
    return;
  }
  auto distance = radius + phage.radius;
  if (geometry::squareDistance(position, phage.position) < distance * distance) {
    auto m = std::min(static_cast<float>(config.simulationInterval * phage.mass), mass - config.cellMinMass);
    modifyMass(-m);
  }
}

void Avatar::interact(Mother& mother)
{
  auto dist = geometry::distance(position, mother.position);
  if (mother.mass >= 1.25 * mass && dist < mother.radius - 0.25 * radius) {
    mother.modifyMass(mass);
    kill();
    player->removeAvatar(this);
  } else if (mass > 1.25 * mother.mass && dist < radius - 0.25 * mother.radius) {
    room.explode(*this);
    mother.kill();
  }
}

bool Avatar::isAttractiveFor(const Avatar& avatar)
{
  return mass * 1.25 < avatar.mass;
}

void Avatar::recombination(float t)
{
  m_recombinationTime += t;
  m_recombined = false;
}

bool Avatar::isRecombined() const
{
  return m_recombined;
}

void Avatar::deflate()
{
  modifyMass(-mass * room.getConfig().player.deflationRatio);
}

void Avatar::annihilate()
{
  auto& cell = room.createVirus();
  cell.modifyMass(mass);
  cell.position = position;
  cell.color = color;
  kill();
}
