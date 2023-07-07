// file   : MySQLConnectionPool.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "MySQLConnectionPool.hpp"

MySQLConnectionPool::MySQLConnectionPool() { }

MySQLConnectionPool::MySQLConnectionPool(const MySQLConfig& config) : m_config(config) { }

MySQLConnectionPool::~MySQLConnectionPool()
{
  clear();
}

void MySQLConnectionPool::init(const MySQLConfig& config)
{
  m_config = config;
}

size_t MySQLConnectionPool::size() const
{
  // TODO: можливо непотокобезпечна операція
  return mysqlpp::ConnectionPool::size();
}

mysqlpp::Connection* MySQLConnectionPool::create()
{
  auto* conn = new mysqlpp::Connection();
  conn->set_option(new mysqlpp::SetCharsetNameOption(m_config.charset));
  conn->set_option(new mysqlpp::ReconnectOption(true));
  conn->connect(
    m_config.database.empty() ? nullptr : m_config.database.c_str(),
    m_config.server.empty() ? nullptr : m_config.server.c_str(),
    m_config.user.empty() ? nullptr : m_config.user.c_str(),
    m_config.password.empty() ? nullptr : m_config.password.c_str(),
    m_config.port
  );
  return conn;
}

void MySQLConnectionPool::destroy(mysqlpp::Connection* conn)
{
  delete conn;
}

unsigned int MySQLConnectionPool::max_idle_time()
{
  return m_config.maxIdleTime;
}
