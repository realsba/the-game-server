// file   : src/entity/Event.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_EVENT_HPP
#define THEGAME_EVENT_HPP

#include <functional>
#include <map>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/post.hpp>

namespace asio = boost::asio;

template<typename... Args>
class Event {
public:
  using Handler = std::function<void(Args...)>;

  explicit Event(const asio::any_io_executor& executor)
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

  void notify(Args&&... args)
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

#endif /* THEGAME_EVENT_HPP */
