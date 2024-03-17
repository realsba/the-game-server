// file   : src/Bot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Bot.hpp"

#include "Room.hpp"

#include "entity/Avatar.hpp"

Bot::Bot(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id)
  : Player(executor, entityFactory, config, id)
  , m_navigationTimer(executor, std::bind_front(&Bot::navigate, this), 200ms)
  , m_respawnTimer(executor)
{
}

Bot::~Bot()
{
  if (m_target) {
    m_target->unsubscribeFromDeath(this);
  }
}

void Bot::start()
{
  m_navigationTimer.start();
}

void Bot::stop()
{
  m_navigationTimer.stop();
  m_respawnTimer.cancel();
}

void Bot::respawn()
{
  Player::respawn();
  m_mainAvatar->modifyMass(static_cast<float>(m_config.bot.mass) - m_mainAvatar->mass);
  m_target = nullptr;
}

void Bot::addAvatar(Avatar* avatar)
{
  Player::addAvatar(avatar);
  m_mainAvatar = findTheBiggestAvatar();
}

void Bot::removeAvatar(Avatar* avatar, Player* killer)
{
  Player::removeAvatar(avatar, killer);
  m_mainAvatar = findTheBiggestAvatar();
  if (!m_status.isAlive) {
    scheduleRespawn();
  }
}

void Bot::navigate()
{
  m_mainAvatar = findTheBiggestAvatar();
  if (m_mainAvatar == nullptr || m_mainAvatar->zombie) {
    return;
  }

  if (m_target && m_target->isAttractiveFor(*m_mainAvatar)) {
    m_pointerOffset = m_target->position - m_position;
    return;
  }

  m_pointerOffset.zero();
  choseTarget();
}

void Bot::choseTarget()
{
  Cell* target = nullptr;
  m_gridmap.query(getViewBox(), [&](Cell& cell) -> bool {
    if (cell.player != this && !cell.zombie && cell.isAttractiveFor(*m_mainAvatar)) {
      if (!target || target->mass < cell.mass) {
        target = &cell;
      }
    }
    return true;
  });

  if (target) {
    m_target = target;
    m_target->subscribeToDeath(this, [&] { m_target = nullptr; });
    startMotion();
  }
}

void Bot::scheduleRespawn()
{
  m_respawnTimer.expires_after(m_config.bot.respawnDelay);
  m_respawnTimer.async_wait(
    [this](const boost::system::error_code& error)
    {
      if (!error) {
        respawn();
      }
    }
  );
}
