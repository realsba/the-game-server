// file   : src/entity/Phage.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_PHAGE_HPP
#define THEGAME_ENTITY_PHAGE_HPP

#include "Cell.hpp"

class Phage : public Cell {
public:
  Phage(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& mass) override;
  void interact(Virus& virus) override;
  void interact(Phage& other) override;
  void interact(Mother& mother) override;
};

#endif /* THEGAME_ENTITY_PHAGE_HPP */
