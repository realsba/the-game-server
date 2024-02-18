// file   : src/MySQLConnectionPool.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_MYSQL_CONNECTION_POOL_HPP
#define THEGAME_MYSQL_CONNECTION_POOL_HPP

#include "Config.hpp"

#include <mysql++/mysql++.h>

class MySQLConnectionPool : public mysqlpp::ConnectionPool {
public:
	MySQLConnectionPool();
	MySQLConnectionPool(const config::MySql& config);
	~MySQLConnectionPool();

  void init(const config::MySql& config);
  size_t size() const;

protected:
	mysqlpp::Connection* create() override;
	void destroy(mysqlpp::Connection* conn) override;
	unsigned int max_idle_time() override;

private:
  config::MySql m_config;
};

#endif /* THEGAME_MYSQL_CONNECTION_POOL_HPP */
