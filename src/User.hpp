// file   : User.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef USER_HPP
#define USER_HPP

#include "UserFwd.hpp"
#include "TimePoint.hpp"
#include "types.hpp"

#include <string>
#include <mutex>

class TSRoom;

class User {
public:
  explicit User(uint32_t id);

  uint32_t getId() const;
  std::string getSessId() const;
  TimePoint getLastAccess() const;
  bool isModified() const;

  ConnectionHdl getConnection() const;
  void setConnection(const ConnectionHdl& hdl);

  TSRoom* getRoom() const;
  void setRoom(TSRoom* room);

  struct Touch {
    void operator ()(const UserSPtr& obj) {
      obj->setLastAccess(TimePoint::clock::now());
    }
  };

private:
  void setSessId(const std::string& v);
  void setLastAccess(const TimePoint& v);

private:
  mutable std::mutex m_mutex;

  ConnectionHdl m_connection;
  std::string   m_sessId;
  std::string   m_name;
  TimePoint     m_lastAccess {TimePoint::clock::now()};
  TSRoom*       m_room {nullptr};
  uint32_t      m_id {0};
  uint8_t       m_color {0};
  bool          m_modified {true};

  friend class UsersCache;
};

#endif /* USER_HPP */
