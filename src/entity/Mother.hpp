// file   : src/entity/Mother.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_MOTHER_HPP
#define THEGAME_ENTITY_MOTHER_HPP

#include "Cell.hpp"

class Mother : public Cell {
public:
  explicit Mother(Room& room, uint32_t id = 0);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& bullet) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;

  uint32_t          foodCount {0};
};

#endif /* THEGAME_ENTITY_MOTHER_HPP */
