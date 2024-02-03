// file   : src/entity/Phage.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_PHAGE_HPP
#define THEGAME_ENTITY_PHAGE_HPP

#include "Cell.hpp"

#include "src/TimePoint.hpp"

class Phage : public Cell {
public:
  explicit Phage(Room& room, uint32_t id = 0);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Mass& mass) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void attract(Avatar& avatar) override;

  TimePoint created {TimePoint::clock::now()};
};

#endif /* THEGAME_ENTITY_PHAGE_HPP */
