// file   : src/ScopeExit.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_SCOPE_EXIT_HPP
#define THEGAME_SCOPE_EXIT_HPP

#include <functional>
#include <utility>

class ScopeExit
{
public:
  using Handler = std::function<void ()>;

  explicit ScopeExit(Handler&& handler) : m_handler(std::move(handler)) { }

  ~ScopeExit()
  {
    if (m_handler) {
      m_handler();
    }
  }

  void cancel()
  {
    m_handler = {};
  }

private:
  Handler m_handler;
};

#endif /* THEGAME_SCOPE_EXIT_HPP */
