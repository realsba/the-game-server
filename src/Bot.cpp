// file   : src/Bot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Bot.hpp"

#include "Room.hpp"

#include "entity/Cell.hpp"

Bot::Bot(const asio::any_io_executor& executor, uint32_t id, Room& room, Gridmap& gridmap)
  : Player(id, room, gridmap)
  , m_navigationTimer(executor, std::bind_front(&Bot::navigate, this), 200ms)
{
}

Bot::~Bot()
{
  if (m_target) {
    m_target->unsubscribeFromDeathEvent(this);
  }
}

void Bot::start()
{
  m_navigationTimer.start();
}

void Bot::stop()
{
  m_navigationTimer.stop();
}

void Bot::init()
{
  Player::init();
  m_target = nullptr;
}

void Bot::navigate()
{
  auto* mainAvatar = getTheBiggestAvatar();
  if (mainAvatar == nullptr || mainAvatar->zombie) { // TODO: revise
    return;
  }

  if (m_target && m_target->isAttractiveFor(*mainAvatar)) {
    m_pointer = m_target->position - m_position;
    return;
  }

  m_pointer.zero();
  choseTarget();
}

void Bot::choseTarget()
{
  auto* mainAvatar = getTheBiggestAvatar();
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
    m_target = target;
    m_target->subscribeToDeathEvent(this, [&](auto *cell) { m_target = nullptr; });
    startMotion();
  }
}

void Bot::startMotion()
{
  for (auto* avatar : m_avatars) {
    avatar->startMotion();
  }
}