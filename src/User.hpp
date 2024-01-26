// file   : User.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_USER_HPP
#define THEGAME_USER_HPP

#include "UserFwd.hpp"
#include "SessionFwd.hpp"
#include "TimePoint.hpp"
#include "types.hpp"

#include <string>
#include <mutex>

class Room;

class User {
public:
  explicit User(uint32_t id);

  uint32_t getId() const;
  std::string getSessId() const;
  TimePoint getLastAccess() const;
  bool isModified() const;

  SessionPtr getSession() const;
  void setSession(const SessionPtr& sess);

  Room* getRoom() const;
  void setRoom(Room* room);

  struct Touch {
    void operator ()(const UserPtr& obj) {
      obj->setLastAccess(TimePoint::clock::now());
    }
  };

private:
  void setSessId(const std::string& v);
  void setLastAccess(const TimePoint& v);

private:
  mutable std::mutex m_mutex;

  SessionPtr    m_session;
  std::string   m_sessId;
  std::string   m_name;
  TimePoint     m_lastAccess {TimePoint::clock::now()};
  Room*         m_room {nullptr};
  uint32_t      m_id {0};
  bool          m_modified {true};

  friend class UsersCache;
};

#endif /* THEGAME_USER_HPP */
