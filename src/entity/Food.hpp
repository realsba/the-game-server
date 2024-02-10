// file   : src/entity/Food.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_FOOD_HPP
#define THEGAME_ENTITY_FOOD_HPP

#include "Cell.hpp"

class Food : public Cell {
public:
  explicit Food(Room& room, uint32_t id = 0);

  bool intersects(const AABB& box) override;

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void attract(Avatar& avatar) override;
  bool isAttractiveFor(const Avatar& avatar) override;
};

#endif /* THEGAME_ENTITY_FOOD_HPP */
