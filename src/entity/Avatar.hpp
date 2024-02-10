// file   : src/entity/Avatar.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_AVATAR_HPP
#define THEGAME_ENTITY_AVATAR_HPP

#include "Cell.hpp"

class Avatar : public Cell {
public:
  explicit Avatar(Room& room, uint32_t id = 0);

  bool shouldBeProcessed() const override;
  void simulate(float dt) override;
  void format(Buffer& buffer) override;

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Mass& mass) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void attract(Avatar& avatar) override;
  bool isAttractiveFor(const Avatar& avatar) override;

  void recombination(float t);
  [[nodiscard]] bool isRecombined() const;

  uint32_t          protection {0};
  float             maxSpeed {0};

private:
  float             m_recombinationTime {0};
  bool              m_recombined {false};
};

#endif /* THEGAME_ENTITY_AVATAR_HPP */
