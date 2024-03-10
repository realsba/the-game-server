// file   : src/entity/Mother.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Mother.hpp"

#include "Avatar.hpp"
#include "Food.hpp"
#include "Bullet.hpp"
#include "Virus.hpp"
#include "Phage.hpp"

#include "src/geometry/geometry.hpp"
#include "src/geometry/AABB.hpp"
#include "src/Gridmap.hpp"
#include "src/Config.hpp"

Mother::Mother(
  const asio::any_io_executor& executor,
  IEntityFactory& entityFactory,
  const config::Room& config,
  uint32_t id
)
  : Cell(executor, entityFactory, config, id)
{
  type = typeMother;
  color = config.mother.color;
  m_foodVelocityDistribution = std::uniform_real_distribution<float>(
    m_config.food.minVelocity, m_config.food.maxVelocity
  );
  m_foodColorIndexDistribution = std::uniform_int_distribution<uint8_t>(
    m_config.food.minColorIndex, m_config.food.maxColorIndex
  );
}

void Mother::interact(Cell& cell)
{
  cell.interact(*this);
}

void Mother::interact(Avatar& avatar)
{
  avatar.interact(*this);
}

void Mother::interact(Food& food)
{
  if (food.creator == this && food.velocity) {
    return;
  }
  if (geometry::squareDistance(position, food.position) < radius * radius) {
    food.kill();
  }
}

void Mother::interact(Bullet& bullet)
{
  auto distance = radius - 0.25 * bullet.radius;
  if (geometry::squareDistance(position, bullet.position) < distance * distance) {
    modifyMass(bullet.mass);
    bullet.kill();
  }
}

void Mother::interact(Virus& virus)
{
  auto distance = radius + virus.radius;
  if (geometry::squareDistance(position, virus.position) < distance * distance) {
    modifyMass(virus.mass);
    virus.kill();
  }
}

void Mother::interact(Phage& phage)
{
  auto distance = radius + phage.radius;
  if (geometry::squareDistance(position, phage.position) < distance * distance) {
    modifyMass(phage.mass);
    phage.kill();
  }
}

void Mother::explode()
{
  auto singleMass = static_cast<int>(m_config.mother.mass);
  auto numObjects = std::max(static_cast<int>(mass) / singleMass - 1, 0);
  if (numObjects > 0) {
    modifyMass(-numObjects * singleMass);
    for (int i = 0; i < numObjects; ++i) {
      auto& obj = m_entityFactory.createMother();
      obj.position = position;
      obj.modifyMass(singleMass);
      obj.modifyVelocity(m_entityFactory.getRandomDirection() * m_config.explodeVelocity);
    }
  }
}

void Mother::generateFood()
{
  if (m_nearbyFoodQuantity >= m_config.mother.nearbyFoodLimit) {
    return;
  }

  auto& generator = m_entityFactory.randomGenerator();
  auto extraMassFactor = std::max(1.0f, (mass - m_config.mother.mass) / m_config.mother.mass);
  auto foodToProduce = static_cast<int>(extraMassFactor * m_config.mother.baseFoodProduction);
  m_nearbyFoodQuantity += foodToProduce;

  for (int i = 0; i < foodToProduce; ++i) {
    const auto& direction = m_entityFactory.getRandomDirection();
    auto& obj = m_entityFactory.createFood();
    obj.creator = this;
    obj.position = position + direction * radius; // TODO: Check if the position is within the bounds of the room
    obj.color = m_foodColorIndexDistribution(generator);
    obj.modifyMass(m_config.food.mass);
    obj.modifyVelocity(direction * m_foodVelocityDistribution(generator));
  }
}

void Mother::calculateNearbyFood()
{
  auto radius = this->radius + m_config.mother.checkRadius;
  Vec2D size(radius, radius);
  AABB boundingBox(position - size, position + size);
  m_nearbyFoodQuantity = m_entityFactory.getGridmap().count(boundingBox);
}
