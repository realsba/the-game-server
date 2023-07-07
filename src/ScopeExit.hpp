// file   : ScopeExit.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef SCOPE_EXIT_HPP
#define SCOPE_EXIT_HPP

#include <functional>

class ScopeExit {
public:
  ScopeExit(std::function<void ()> f) : m_Function(f) { }

  ~ScopeExit()
  {
    m_Function();
  }

private:
  std::function<void ()> m_Function;
};

#endif /* SCOPE_EXIT_HPP */
