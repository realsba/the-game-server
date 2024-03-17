// file   : src/PlayerFwd.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_PLAYER_FWD_HPP
#define THEGAME_PLAYER_FWD_HPP

#include <memory>

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerWPtr = std::weak_ptr<Player>;

#endif /* THEGAME_PLAYER_FWD_HPP */
