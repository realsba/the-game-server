// file   : src/entity/Bullet.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Bullet.hpp"

#include "Avatar.hpp"
#include "Virus.hpp"
#include "Phage.hpp"
#include "Mother.hpp"

Bullet::Bullet(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : Cell(executor, entityFactory, config, id)
{
  type = typeMass;
}

void Bullet::interact(Cell& cell)
{
  cell.interact(*this);
}

void Bullet::interact(Avatar& avatar)
{
  avatar.interact(*this);
}

void Bullet::interact(Virus& virus)
{
  virus.interact(*this);
}

void Bullet::interact(Phage& phage)
{
  phage.interact(*this);
}

void Bullet::interact(Mother& mother)
{
  mother.interact(*this);
}

bool Bullet::isAttractiveFor(const Avatar& avatar)
{
  return !zombie;
}
