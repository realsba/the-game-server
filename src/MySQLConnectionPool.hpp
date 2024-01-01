// file   : MySQLConnectionPool.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_MYSQL_CONNECTION_POOL_HPP
#define THEGAME_MYSQL_CONNECTION_POOL_HPP

#include "MySQLConfig.hpp"

#include <mysql++/mysql++.h>

class MySQLConnectionPool : public mysqlpp::ConnectionPool {
public:
	MySQLConnectionPool();
	MySQLConnectionPool(const MySQLConfig& config);
	~MySQLConnectionPool();

  void init(const MySQLConfig& config);
  size_t size() const;

protected:
	mysqlpp::Connection* create() override;
	void destroy(mysqlpp::Connection* conn) override;
	unsigned int max_idle_time() override;

private:
  MySQLConfig m_config;
};

#endif /* THEGAME_MYSQL_CONNECTION_POOL_HPP */
