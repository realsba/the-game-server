// file   : src/entity/Avatar.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Avatar.hpp"

#include "src/packet/serialization.hpp"

#include "src/Player.hpp"
#include "src/Room.hpp"

Avatar::Avatar(Room& room, uint32_t id)
  : Cell(room, id)
{
  type = typeAvatar;
}

void Avatar::simulate(float dt)
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

void Avatar::interact(Avatar& avatar)
{
  room.interact(*this, avatar);
}

void Avatar::interact(Food& food)
{
  room.interact(*this, food);
}

void Avatar::interact(Mass& mass)
{
  room.interact(*this, mass);
}

void Avatar::interact(Virus& virus)
{
  room.interact(*this, virus);
}

void Avatar::interact(Phage& phage)
{
  room.interact(*this, phage);
}

void Avatar::interact(Mother& mother)
{
  room.interact(*this, mother);
}

void Avatar::attract(Avatar& avatar)
{
  room.attract(avatar, *this);
}

bool Avatar::isAttractive(const Avatar& avatar)
{
  return mass < 1.25 * avatar.mass;
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
