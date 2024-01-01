// file   : Application.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_APPLICATION_HPP
#define THEGAME_APPLICATION_HPP

#include "Config.hpp"

#include "ListenerFwd.hpp"

#include "MySQLConnectionPool.hpp"
#include "RoomManager.hpp"
#include "UsersCache.hpp"
#include "Listener.hpp"
#include "Session.hpp"
#include "Timer.hpp"

#include "packet/InputPacketTypes.hpp"
#include "packet/Packet.hpp"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <map>

using namespace std::placeholders;

class Application : private boost::noncopyable {
public:
  explicit Application(std::string configFileName);

  void start();
  void stop();
  void save();
  void info();

private:
  void sessionMessageHandler(const SessionPtr& sess, beast::flat_buffer& buffer);
  void sessionOpenHandler(const SessionPtr& sess);
  void sessionCloseHandler(const SessionPtr& sess);

  void actionPing(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionGreeting(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionPlay(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionSpectate(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionPointer(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionEject(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionSplit(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionChatMessage(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionWatch(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);
  void actionPaint(const UserSPtr& user, const SessionPtr& sess, Deserializer& request);

  void update();
  void checkConnections();
  void statistic();

private:
  using Handler = std::function<void(const UserSPtr&, const SessionPtr& sess, Deserializer& ms)>;
  using Handlers = std::map<int, Handler>;

  const Handlers m_handlers {
    { InputPacketTypes::Ping,         std::bind(&Application::actionPing,         this, _1, _2, _3) },
    { InputPacketTypes::Greeting,     std::bind(&Application::actionGreeting,     this, _1, _2, _3) },
    { InputPacketTypes::Play,         std::bind(&Application::actionPlay,         this, _1, _2, _3) },
    { InputPacketTypes::Spectate,     std::bind(&Application::actionSpectate,     this, _1, _2, _3) },
    { InputPacketTypes::Pointer,      std::bind(&Application::actionPointer,      this, _1, _2, _3) },
    { InputPacketTypes::Eject,        std::bind(&Application::actionEject,        this, _1, _2, _3) },
    { InputPacketTypes::Split,        std::bind(&Application::actionSplit,        this, _1, _2, _3) },
    { InputPacketTypes::ChatMessage,  std::bind(&Application::actionChatMessage,  this, _1, _2, _3) },
    { InputPacketTypes::Watch,        std::bind(&Application::actionWatch,        this, _1, _2, _3) },
    { InputPacketTypes::Paint,        std::bind(&Application::actionPaint,        this, _1, _2, _3) },
  };

  mutable std::mutex            m_mutex;
  asio::io_context              m_ioContext;
  asio::ip::udp::socket         m_influxdb {m_ioContext};
  MySQLConnectionPool           m_mysqlConnectionPool;
  Sessions                      m_sessions;
  UsersCache                    m_users {m_mysqlConnectionPool};
  RoomManager                   m_roomManager;
  std::vector<std::thread>      m_threads;
  std::string                   m_configFileName {"application.conf"};
  Config                        m_config;
  Timer                         m_timer;
  std::size_t                   m_maxSessions {0};
  uint32_t                      m_registrations {0};
  const size_t                  m_nameMaxLength {16};
  bool                          m_started {false};

  ListenerPtr                   m_listener;
};

#endif /* THEGAME_APPLICATION_HPP */
