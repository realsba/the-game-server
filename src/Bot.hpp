// file   : src/Bot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_BOT_HPP
#define THEGAME_BOT_HPP

#include "Player.hpp"

#include "Timer.hpp"

#include <boost/asio/any_io_executor.hpp>

namespace asio = boost::asio;

class Bot : public Player {
public:
  Bot(const asio::any_io_executor& executor, IEntityFactory& entityFactory, const config::Room& config, uint32_t id);
  ~Bot() override;

  void start();
  void stop();

  void respawn() override;

protected:
  void addAvatar(Avatar* avatar) override;
  void removeAvatar(Avatar* avatar) override;

  void navigate();
  void choseTarget();
  void scheduleRespawn();

private:
  Timer                 m_navigationTimer;
  asio::steady_timer    m_respawnTimer;
  Avatar*               m_mainAvatar {nullptr};
  Cell*                 m_target {nullptr};
};

#endif /* THEGAME_BOT_HPP */
