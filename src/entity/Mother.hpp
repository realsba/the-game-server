// file   : entity/Mother.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef ENTITY_MOTHER_HPP
#define ENTITY_MOTHER_HPP

#include "Cell.hpp"

#include "src/TimePoint.hpp"

class Mother : public Cell {
public:
  Mother(Room& room);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Mass& mass) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;

  void magnetism(Avatar& avatar) override;

  TimePoint created {TimePoint::clock::now()};
  uint32_t foodCount {0};
  float startRadius {0};
};

#endif /* ENTITY_MOTHER_HPP */
