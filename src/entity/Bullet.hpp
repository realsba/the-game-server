// file   : src/entity/Bullet.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_BULLET_HPP
#define THEGAME_ENTITY_BULLET_HPP

#include "Cell.hpp"

class Bullet : public Cell {
public:
  explicit Bullet(Room& room, uint32_t id = 0);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;

  bool isAttractiveFor(const Avatar& avatar) override;
};

#endif /* THEGAME_ENTITY_BULLET_HPP */
