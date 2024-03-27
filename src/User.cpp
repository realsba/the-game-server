// file   : src/User.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "User.hpp"

User::User(uint32_t id)
  : m_id(id)
{
}

uint32_t User::getId() const
{
  return m_id;
}

std::string User::getToken() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_token;
}

void User::setToken(const std::string& v)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_token = v;
  m_modified = true;
}

TimePoint User::getLastAccess() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_lastAccess;
}

bool User::isModified() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_modified;
}

void User::setLastAccess(const TimePoint& v)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_lastAccess = v;
}

SessionPtr User::getSession() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_session;
}

void User::setSession(const SessionPtr& sess)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_session = sess;
}
