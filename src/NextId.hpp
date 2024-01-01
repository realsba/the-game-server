// file   : NextId.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_NEXT_ID_HPP
#define THEGAME_NEXT_ID_HPP

#include <cstdint>
#include <vector>
#include <stack>

class NextId {
public:
  void push(uint32_t id);
  uint32_t pop();

private:
  using UnusedIds = std::stack<uint32_t, std::vector<uint32_t>>;

  UnusedIds m_unusedIds;
  uint32_t  m_nextId {1};
};

#endif /* THEGAME_NEXT_ID_HPP */
