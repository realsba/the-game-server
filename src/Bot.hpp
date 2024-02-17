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
  Bot(const asio::any_io_executor& executor, uint32_t id, Room& room, Gridmap& gridmap);
  ~Bot() override;

  void start();
  void stop();

  void init() override;

protected:
  void navigate();
  void choseTarget();

private:
  Timer m_navigationTimer;
  Cell* m_target {nullptr};
};

#endif /* THEGAME_BOT_HPP */
