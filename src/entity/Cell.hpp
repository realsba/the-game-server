// file   : src/entity/Cell.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_CELL_HPP
#define THEGAME_ENTITY_CELL_HPP

#include "src/IEntityFactory.hpp"

#include "src/geometry/Circle.hpp"
#include "src/TimePoint.hpp"
#include "src/Event.hpp"
#include "src/types.hpp"

#include <string>

#include <boost/asio/any_io_executor.hpp>

namespace asio = boost::asio;

class Sector;
class Player;
class AABB;

namespace config {
  class Room;
}

class Cell : public Circle {
public:
  static constexpr auto MIN_MASS = 1.0f;

  Cell(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);
  virtual ~Cell() = default;

  [[nodiscard]] AABB getAABB() const;

  virtual void modifyMass(float value);
  void modifyVelocity(const Vec2D& value);

  void applyResistanceForce();

  virtual bool intersects(const AABB& box);
  virtual void simulate(double dt);
  virtual void format(Buffer& buffer);

  virtual void interact(Cell& cell);
  virtual void interact(Avatar& avatar);
  virtual void interact(Food& food);
  virtual void interact(Bullet& bullet);
  virtual void interact(Virus& virus);
  virtual void interact(Phage& phage);
  virtual void interact(Mother& mother);

  virtual bool isAttractiveFor(const Avatar& avatar);

  void kill();
  void startMotion();
  void subscribeToDeathEvent(void* tag, Event<>::Handler&& handler);
  void unsubscribeFromDeathEvent(void* tag);
  void subscribeToMassChangeEvent(void* tag, Event<float>::Handler&& handler);
  void unsubscribeFromMassChangeEvent(void* tag);
  void subscribeToMotionStartedEvent(void* tag, Event<>::Handler&& handler);
  void unsubscribeFromMotionStartedEvent(void* tag);
  void subscribeToMotionStoppedEvent(void* tag, Event<>::Handler&& handler);
  void unsubscribeFromMotionStoppedEvent(void* tag);

  enum Type {
    typeAvatar = 1,
    typeFood = 2,
    typeMass = 3,
    typeVirus = 4,
    typePhage = 5,
    typeMother = 6,

    isNew = 64,
    isMoving = 128
  };

  std::set<Sector*> sectors;
  Sector*           leftTopSector {nullptr};
  Sector*           rightBottomSector {nullptr};
  TimePoint         created {TimePoint::clock::now()};
  Vec2D             velocity;
  Vec2D             force;
  Cell*             creator {nullptr};
  Player*           player {nullptr};
  float             mass {0};
  float             resistanceRatio {0};
  uint32_t          id {0};
  uint32_t          type {0};
  uint8_t           color {0};
  bool              newly {true};
  bool              zombie {false};
  bool              materialPoint {false};

protected:
  const config::Room&   m_config;
  IEntityFactory&       m_entityFactory;

private:
  Event<>               m_deathEvent;
  Event<float>          m_massChangeEvent;
  Event<>               m_motionStartedEvent;
  Event<>               m_motionStoppedEvent;
};

#endif /* THEGAME_ENTITY_CELL_HPP */
