// file   : src/entity/Mother.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_MOTHER_HPP
#define THEGAME_ENTITY_MOTHER_HPP

#include "Cell.hpp"

class Mother : public Cell {
public:
  Mother(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& bullet) override;
  void interact(Virus& virus) override;
  void interact(Phage& phage) override;

  void explode();

  uint32_t foodCount {0};
};

#endif /* THEGAME_ENTITY_MOTHER_HPP */
