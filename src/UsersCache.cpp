// file   : src/UsersCache.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "UsersCache.hpp"

#include "ScopeExit.hpp"
#include "util.hpp"

#include <fmt/chrono.h>
#include <mysql++/ssqls.h>

sql_create_3(DboUserCreate, 1, 0,
  mysqlpp::sql_varchar, token,
  mysqlpp::sql_datetime, created,
  mysqlpp::sql_int_unsigned, ip
)

sql_create_2(DboUser, 1, 0,
  mysqlpp::sql_int_unsigned, id,
  mysqlpp::sql_varchar, token
)

UsersCache::UsersCache(MySQLConnectionPool& pool)
  : m_mysqlConnectionPool(pool)
{
  DboUserCreate::table("users");
  DboUser::table("users");
}

void UsersCache::save()
{
  std::lock_guard lock(m_mutex);
  if (m_items.empty()) {
    return;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  DboUser dbo;
  for (const UserPtr& item : m_items) {
    DboUser orig;
    orig.id = item->getId();
    dbo.id = orig.id;
    dbo.token = item->getToken();
    query.update(orig, dbo);
    query.execute();
  }
}

UserPtr UsersCache::create(uint32_t ip)
{
  std::lock_guard lock(m_mutex);
  auto& ind = m_items.get<ByToken>();
  DboUserCreate dbo;
  dbo.created = mysqlpp::String(fmt::to_string(std::chrono::system_clock::now()));
  uint32_t id = 0;
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([] { mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  do {
    dbo.token = randomString(32);
    if (ind.find(dbo.token) != ind.end()) {
      continue;
    }
    dbo.ip = ip;
    auto query = db->query();
    query.insert(dbo);
    if (query.execute().rows()) {
      id = query.insert_id();
    }
  } while (!id);
  const auto& user = std::make_shared<User>(id);
  user->setToken(dbo.token);
  if (!m_items.emplace(user).second) {
    throw std::runtime_error("Bad insert new user into the users cache");
  }
  return user;
}

UserPtr UsersCache::getUserById(uint32_t id)
{
  std::lock_guard lock(m_mutex);
  const auto& it = m_items.find(id);
  if (it != m_items.end()) {
    m_items.modify(it, User::Touch());
    return *it;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([] { mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  query << "SELECT id,token FROM users WHERE id=" << mysqlpp::quote_only << id;
  auto res = query.store();
  if (!res.empty()) {
    const DboUser& dbo = res[0];
    const auto& user = std::make_shared<User>(dbo.id);
    user->setToken(dbo.token);
    if (!m_items.emplace(user).second) {
      throw std::runtime_error("Bad insert user into the users cache by id");
    }
    return user;
  }
  return {};
}

UserPtr UsersCache::getUserByToken(const std::string& sid)
{
  std::lock_guard lock(m_mutex);
  auto& ind = m_items.get<ByToken>();
  if (const auto& it = ind.find(sid); it != ind.end()) {
    ind.modify(it, User::Touch());
    return *it;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([] { mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  query << "SELECT id,token FROM users WHERE token=" << mysqlpp::quote_only << sid;
  if (auto res = query.store(); !res.empty()) {
    const DboUser& dbo = res[0];
    const auto& user = std::make_shared<User>(dbo.id);
    user->setToken(dbo.token);
    if (!m_items.emplace(user).second) {
      throw std::runtime_error("Bad insert user into the users cache by token");
    }
    return user;
  }
  return {};
}
