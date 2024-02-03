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

  Cell* eatTarget = nullptr;
  m_gridmap.query(getViewBox(), [&](Cell& target) -> bool {
    if (target.player != this && !target.zombie && target.isAttractiveFor(*mainAvatar)) {
      if (!eatTarget || eatTarget->mass < target.mass) {
        eatTarget = &target;
      }
    }
    return true;
  });

  if (eatTarget) {
    m_pointer = eatTarget->position - m_position;
  }

  m_target = eatTarget;
}
