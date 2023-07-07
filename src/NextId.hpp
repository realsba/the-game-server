// file   : NextId.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef NEXTID_HPP
#define NEXTID_HPP

#include <cstdint>
#include <vector>
#include <stack>

class NextId {
public:
  void push(uint32_t id);
  uint32_t pop();

private:
  typedef std::stack<uint32_t, std::vector<uint32_t>> UnusedIds;

  UnusedIds m_unusedIds;
  uint32_t  m_nextId {1};
};

#endif /* NEXTID_HPP */
