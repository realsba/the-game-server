// file   : src/UsersCache.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "UsersCache.hpp"

#include "ScopeExit.hpp"
#include "util.hpp"

#include <fmt/chrono.h>
#include <mysql++/ssqls.h>

sql_create_3(DboUserCreate, 1, 0,
  mysqlpp::sql_varchar, sessId,
  mysqlpp::sql_datetime, created,
  mysqlpp::sql_int_unsigned, ip
)

sql_create_4(DboUser, 1, 0,
  mysqlpp::sql_int_unsigned, id,
  mysqlpp::sql_varchar, sessId,
  mysqlpp::sql_varchar, email,
  mysqlpp::sql_varchar, password
)

UsersCache::UsersCache(MySQLConnectionPool& pool)
  : m_mysqlConnectionPool(pool)
  , m_ttl(std::chrono::minutes(60))
{
  DboUserCreate::table("users");
  DboUser::table("users");
}

void UsersCache::setTtl(const Duration& v)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_ttl = v;
}

Duration UsersCache::getTtl() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_ttl;
}

void UsersCache::save()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_items.empty()) {
    return;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  DboUser orig, dbo;
  for (const UserPtr& item : m_items) {
    orig.id = item->getId();
    dbo.id = orig.id;
    dbo.sessId = item->getSessId();
    query.update(orig, dbo);
    query.execute();
  }
}

UserPtr UsersCache::create(uint32_t ip)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& ind = m_items.get<BySessId>();
  DboUserCreate dbo;
  dbo.created = mysqlpp::String(fmt::to_string(std::chrono::system_clock::now()));
  uint32_t id = 0;
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  do {
    dbo.sessId = randomString(32);
    if (ind.find(dbo.sessId) != ind.end()) {
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
  user->setSessId(dbo.sessId);
  if (!m_items.emplace(user).second) {
    throw std::runtime_error("Bad insert new user into the users cache");
  }
  return user;
}

UserPtr UsersCache::getUserById(uint32_t id)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  const auto& it = m_items.find(id);
  if (it != m_items.end()) {
    m_items.modify(it, User::Touch());
    return *it;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  query << "SELECT id,sessId FROM users WHERE id=" << mysqlpp::quote_only << id;
  auto res = query.store();
  if (!res.empty()) {
    const DboUser& dbo = res[0];
    const auto& user = std::make_shared<User>(dbo.id);
    user->setSessId(dbo.sessId);
    if (!m_items.emplace(user).second) {
      throw std::runtime_error("Bad insert user into the users cache by id");
    }
    return user;
  }
  return {};
}

UserPtr UsersCache::getUserBySessId(const std::string& sid)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto& ind = m_items.get<BySessId>();
  const auto& it = ind.find(sid);
  if (it != ind.end()) {
    ind.modify(it, User::Touch());
    return *it;
  }
  mysqlpp::Connection::thread_start();
  ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
  mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
  auto query = db->query();
  query << "SELECT id,sessId FROM users WHERE sessId=" << mysqlpp::quote_only << sid;
  auto res = query.store();
  if (!res.empty()) {
    const DboUser& dbo = res[0];
    const auto& user = std::make_shared<User>(dbo.id);
    user->setSessId(dbo.sessId);
    if (!m_items.emplace(user).second) {
      throw std::runtime_error("Bad insert user into the users cache by sessId");
    }
    return user;
  }
  return {};
}
