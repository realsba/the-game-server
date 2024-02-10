// file   : src/entity/Event.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_EVENT_HPP
#define THEGAME_EVENT_HPP

#include <functional>
#include <map>

template<typename... Args>
class Event {
public:
  using Handler = std::function<void(Args...)>;

  void subscribe(void* tag, Handler&& handler) {
    m_subscribers.emplace(tag, std::forward<Handler>(handler));
  }

  void unsubscribe(void* tag) {
    m_subscribers.erase(tag);
  }

  void notify(Args&&... args) {
    for (const auto& it : m_subscribers) {
      it.second(std::forward<Args>(args)...);
    }
  }

private:
  std::map<void*, Handler> m_subscribers;
};

#endif /* THEGAME_EVENT_HPP */
