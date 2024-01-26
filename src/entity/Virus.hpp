// file   : entity/Virus.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_VIRUS_HPP
#define THEGAME_ENTITY_VIRUS_HPP

#include "Cell.hpp"

#include "src/TimePoint.hpp"

class Virus : public Cell {
public:
  explicit Virus(Room& room);

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

#endif /* THEGAME_ENTITY_VIRUS_HPP */
