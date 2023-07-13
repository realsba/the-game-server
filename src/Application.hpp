// file   : Application.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Config.hpp"

#include "MySQLConnectionPool.hpp"
#include "WebsocketServer.hpp"
#include "RoomManager.hpp"
#include "UsersCache.hpp"
#include "Timer.hpp"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <map>

class MemoryStream;

class Application : private boost::noncopyable {
public:
  explicit Application(std::string configFileName);

  void start();
  void stop();
  void save();
  void info();

private:
  void openHandler(ConnectionHdl hdl);
  void closeHandler(ConnectionHdl hdl);
  void messageHandler(ConnectionHdl hdl, WebsocketServer::message_ptr msg);

  void actionPing(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionGreeting(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionPlay(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionSpectate(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionPointer(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionEject(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionSplit(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionChatMessage(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionWatch(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);
  void actionPaint(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request);

  void update();
  void checkConnections();
  void statistic();

private:
  typedef std::function<void(const UserSPtr&, const ConnectionHdl&, MemoryStream&)> Handler;
  typedef std::map<int, Handler> Handlers;

  mutable std::mutex                    m_mutex;
  boost::asio::io_context               m_ioService;
  boost::asio::ip::udp::socket          m_influxdb {m_ioService};
  WebsocketServer                       m_websocketServer;
  MySQLConnectionPool                   m_mysqlConnectionPool;
  Connections                           m_connections;
  UsersCache                            m_users {m_mysqlConnectionPool};
  RoomManager                           m_roomManager {m_ioService, m_websocketServer};
  std::vector<std::thread>              m_threads;
  std::string                           m_configFileName {"application.conf"};
  Config                                m_config;
  Timer                                 m_timer;
  uint32_t                              m_maxConnections {0};
  uint32_t                              m_registrations {0};
  const size_t                          m_nameMaxLength {16};
  bool                                  m_started {false};

// не захищені мютексом дані
  Handlers m_handlers;
};

#endif /* APPLICATION_HPP */
