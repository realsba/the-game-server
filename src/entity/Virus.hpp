// file   : src/entity/Virus.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_VIRUS_HPP
#define THEGAME_ENTITY_VIRUS_HPP

#include "Cell.hpp"

class Virus : public Cell {
public:
  explicit Virus(Room& room, uint32_t id = 0);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Mass& mass) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  void attract(Avatar& avatar) override;
};

#endif /* THEGAME_ENTITY_VIRUS_HPP */
