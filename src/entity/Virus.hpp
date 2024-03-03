// file   : src/entity/Virus.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_ENTITY_VIRUS_HPP
#define THEGAME_ENTITY_VIRUS_HPP

#include "Cell.hpp"

class Virus : public Cell {
public:
  Virus(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);

  void interact(Cell& cell) override;
  void interact(Avatar& avatar) override;
  void interact(Food& food) override;
  void interact(Bullet& bullet) override;
  void interact(Virus& other) override;
  void interact(Phage& phage) override;
  void interact(Mother& mother) override;
};

#endif /* THEGAME_ENTITY_VIRUS_HPP */
