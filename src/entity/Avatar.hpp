// file   : src/entity/Avatar.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_AVATAR_HPP
#define THEGAME_ENTITY_AVATAR_HPP

#include "Cell.hpp"

class Avatar : public Cell {
public:
  Avatar(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);

  void modifyMass(float value) override;

  void simulate(double dt) override;
  void format(Buffer& buffer) override;

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& bullet) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  bool isAttractiveFor(const Avatar& avatar) override;

  void eject(const Vec2D& point);
  bool split(const Vec2D& point);
  void recombination(float t);
  [[nodiscard]] bool isRecombined() const;

  void deflate();
  void annihilate();
  void explode();

  uint32_t          protection {0};
  float             maxVelocity {0};

private:
  float             m_recombinationTime {0};
  bool              m_recombined {false};
};

#endif /* THEGAME_ENTITY_AVATAR_HPP */
