// file   : entity/Food.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_FOOD_HPP
#define THEGAME_ENTITY_FOOD_HPP

#include "Cell.hpp"

class Food : public Cell {
public:
  explicit Food(Room& room);

  bool intersects(const AABB& box) override;

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void magnetism(Avatar& avatar) override;
};

#endif /* THEGAME_ENTITY_FOOD_HPP */
