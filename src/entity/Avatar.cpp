// file   : entity/Avatar.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Avatar.hpp"

#include "src/MemoryStream.hpp"
#include "src/Player.hpp"
#include "src/Room.hpp"

Avatar::Avatar(Room& room) :
  Cell(room)
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

void Avatar::format(MemoryStream& ms)
{
  bool moving = static_cast<bool>(velocity);
  ms.writeUInt8(type | typeNew * newly | typeMoving * moving);
  ms.writeUInt32(id);
  ms.writeFloat(position.x);
  ms.writeFloat(position.y);
  ms.writeUInt32(mass);
  ms.writeUInt16(radius);
  ms.writeUInt8(color);
  ms.writeUInt32(player->getId());
  // ms.writeUInt32(protection);
  if (moving) {
    ms.writeFloat(velocity.x);
    ms.writeFloat(velocity.y);
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

void Avatar::magnetism(Avatar& avatar)
{
  room.magnetism(avatar, *this);
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
