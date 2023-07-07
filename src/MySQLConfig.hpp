// file   : MySQLConfig.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef MYSQL_CONFIG_HPP
#define MYSQL_CONFIG_HPP

#include <string>

class MySQLConfig {
public:
  std::string database;
  std::string server;
  std::string user;
  std::string password;
  std::string charset;
  uint        port {0};
  uint        maxIdleTime {0};
};

#endif /* MYSQL_CONFIG_HPP */
