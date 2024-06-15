// file   : src/UsersCache.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_USERS_CACHE_HPP
#define THEGAME_USERS_CACHE_HPP

#include "MySQLConnectionPool.hpp"
#include "User.hpp"

#if !defined(NDEBUG)
  #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
  #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <string>
#include <mutex>

class UsersCache {
public:
  explicit UsersCache(MySQLConnectionPool& pool);

  void save();

  UserPtr create(uint32_t ip);
  UserPtr getUserById(uint32_t id);
  UserPtr getUserByToken(const std::string& sid);

protected:
  struct ById { };
  struct ByToken { };
  struct ByLastAccess { };

  using Items = boost::multi_index::multi_index_container<
    UserPtr,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<ById>,
        boost::multi_index::const_mem_fun<User, uint32_t, &User::getId>
      >,
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<ByToken>,
        boost::multi_index::const_mem_fun<User, std::string, &User::getToken>
      >,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<ByLastAccess>,
        boost::multi_index::const_mem_fun<User, TimePoint, &User::getLastAccess>
      >
    >
  >;

  MySQLConnectionPool&          m_mysqlConnectionPool;
  mutable std::mutex            m_mutex;
  Items                         m_items;
};

#endif /* THEGAME_USERS_CACHE_HPP */
