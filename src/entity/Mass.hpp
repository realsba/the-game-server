// file   : entity/Mass.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef ENTITY_MASS_HPP
#define ENTITY_MASS_HPP

#include "Cell.hpp"

class Mass : public Cell {
public:
  Mass(Room& room);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void magnetism(Avatar& avatar) override;
  bool isAttractive(const Avatar& avatar) override;
};

#endif /* ENTITY_MASS_HPP */
