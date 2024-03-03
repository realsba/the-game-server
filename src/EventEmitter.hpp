// file   : src/entity/EventEmitter.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_EVENT_EMITTER_HPP
#define THEGAME_EVENT_EMITTER_HPP

#include <functional>
#include <map>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/post.hpp>

namespace asio = boost::asio;

template<typename... Args>
class EventEmitter {
public:
  using Handler = std::function<void(Args...)>;

  explicit EventEmitter(const asio::any_io_executor& executor)
    : m_executor(executor)
  {}

  void subscribe(void* tag, Handler&& handler)
  {
    m_subscribers.emplace(tag, std::forward<Handler>(handler));
  }

  void unsubscribe(void* tag)
  {
    m_subscribers.erase(tag);
  }

  void emit(Args&&... args)
  {
    for (const auto& it : m_subscribers) {
      asio::post(m_executor,
        [func = it.second, ...args = std::forward<Args>(args)]() mutable { func(std::forward<Args>(args)...); }
      );
    }
  }

  void clear()
  {
    m_subscribers.clear();
  }

private:
  const asio::any_io_executor&  m_executor;
  std::map<void*, Handler>      m_subscribers;
};

#endif /* THEGAME_EVENT_EMITTER_HPP */
