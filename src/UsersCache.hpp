// file   : UsersCache.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef USERS_CACHE_HPP
#define USERS_CACHE_HPP

#include "MySQLConnectionPool.hpp"
#include "TimePoint.hpp"
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
  UsersCache(MySQLConnectionPool& pool);

  void setTtl(const Duration& v);
  Duration getTtl() const;
  void save();
  void flush();
  void clear();

  UserSPtr create(uint32_t ip);
  UserSPtr getUserById(uint32_t id);
  UserSPtr getUserBySessId(const std::string& sid);

protected:
  struct ById { };
  struct BySessId { };
  struct ByLastAccess { };

  typedef boost::multi_index::multi_index_container<
    UserSPtr,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<ById>,
        boost::multi_index::const_mem_fun<User, uint32_t, &User::getId>
      >,
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<BySessId>,
        boost::multi_index::const_mem_fun<User, std::string, &User::getSessId>
      >,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<ByLastAccess>,
        boost::multi_index::const_mem_fun<User, TimePoint, &User::getLastAccess>
      >
    >
  > Items;

protected:
  MySQLConnectionPool& m_mysqlConnectionPool;
  mutable std::mutex m_mutex;
  Items m_items;
  Duration m_ttl;
};

#endif /* USERS_CACHE_HPP */
