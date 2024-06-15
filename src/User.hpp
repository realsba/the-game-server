// file   : src/User.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_USER_HPP
#define THEGAME_USER_HPP

#include "UserFwd.hpp"
#include "SessionFwd.hpp"
#include "TimePoint.hpp"

#include <string>
#include <mutex>

class User {
public:
  explicit User(uint32_t id);

  uint32_t getId() const;
  std::string getToken() const;
  TimePoint getLastAccess() const;
  bool isModified() const;

  SessionPtr getSession() const;
  void setSession(const SessionPtr& sess);

  struct Touch {
    void operator ()(const UserPtr& obj) const
    {
      obj->setLastAccess(TimePoint::clock::now());
    }
  };

private:
  void setToken(const std::string& v);
  void setLastAccess(const TimePoint& v);

  mutable std::mutex  m_mutex;
  SessionPtr          m_session;
  std::string         m_token;
  std::string         m_name;
  TimePoint           m_lastAccess {TimePoint::clock::now()};
  const uint32_t      m_id {0};
  bool                m_modified {true};

  friend class UsersCache;
};

#endif /* THEGAME_USER_HPP */
