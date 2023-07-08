// file   : ScopeExit.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef SCOPE_EXIT_HPP
#define SCOPE_EXIT_HPP

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
    m_handler = Handler();
  }

private:
  Handler m_handler;
};

#endif /* SCOPE_EXIT_HPP */
