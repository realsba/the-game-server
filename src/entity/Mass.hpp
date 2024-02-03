// file   : src/entity/Mass.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_MASS_HPP
#define THEGAME_ENTITY_MASS_HPP

#include "Cell.hpp"

class Mass : public Cell {
public:
  explicit Mass(Room& room, uint32_t id = 0);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void attract(Avatar& avatar) override;
  bool isAttractive(const Avatar& avatar) override;
};

#endif /* THEGAME_ENTITY_MASS_HPP */
