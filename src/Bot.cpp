// file   : src/Bot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Bot.hpp"

#include "Room.hpp"

#include "entity/Cell.hpp"

Bot::Bot(uint32_t id, Room& room, Gridmap& gridmap)
  : Player(id, room, gridmap)
{
}

void Bot::init()
{
  Player::init();
  m_target = nullptr;
}

void Bot::choseTarget()
{
  auto* mainAvatar = getTheBiggestAvatar();
  if (mainAvatar == nullptr || mainAvatar->zombie) {
    return;
  }

  if (m_target) {
    m_pointer = m_target->position - m_position;
    return;
  }

  Cell* target = nullptr;
  m_gridmap.query(getViewBox(), [&](Cell& cell) -> bool {
    if (cell.player != this && !cell.zombie && cell.isAttractiveFor(*mainAvatar)) {
      if (!target || target->mass < cell.mass) {
        target = &cell;
      }
    }
    return true;
  });

  if (target) {
    m_pointer = target->position - m_position;
  }

  m_target = target;
}
