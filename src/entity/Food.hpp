// file   : entity/Food.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef ENTITY_FOOD_HPP
#define ENTITY_FOOD_HPP

#include "Cell.hpp"

class Food : public Cell {
public:
  Food(Room& room);

  bool intersects(const AABB& box);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void magnetism(Avatar& avatar) override;
};

#endif /* ENTITY_FOOD_HPP */
