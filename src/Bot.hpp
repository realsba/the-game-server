// file   : src/Bot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_BOT_HPP
#define THEGAME_BOT_HPP

#include "Player.hpp"

class Bot : public Player {
public:
  Bot(uint32_t id, Room& room, Gridmap& gridmap);

  void init() override;

  void choseTarget();

private:
  Cell* m_target {nullptr};
};

#endif /* THEGAME_BOT_HPP */
