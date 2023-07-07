// file   : NextId.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "NextId.hpp"

void NextId::push(uint32_t id)
{
  m_unusedIds.push(id);
}

uint32_t NextId::pop()
{
  uint32_t id;
  if (!m_unusedIds.empty()) {
    id = m_unusedIds.top();
    m_unusedIds.pop();
  } else {
    id = m_nextId++;
  }
  return id;
}
