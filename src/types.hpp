// file   : types.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_TYPES_HPP
#define THEGAME_TYPES_HPP

#include "SessionFwd.hpp"

#include <set>

using Sessions = std::set<SessionPtr>;

#include <vector>
using Buffer = std::vector<char>;
using BufferPtr = std::shared_ptr<Buffer>;

#endif /* THEGAME_TYPES_HPP */
