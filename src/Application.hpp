// file   : src/Application.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_APPLICATION_HPP
#define THEGAME_APPLICATION_HPP

#include "ListenerFwd.hpp"

#include "Config.hpp"
#include "IOThreadPool.hpp"
#include "IncomingPacket.hpp"
#include "Listener.hpp"
#include "MySQLConnectionPool.hpp"
#include "RoomManager.hpp"
#include "Session.hpp"
#include "Timer.hpp"
#include "UsersCache.hpp"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include <map>
#include <mutex>
#include <string>
#include <thread>

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

  void actionPing(const SessionPtr& sess, beast::flat_buffer& request);
  void actionGreeting(const SessionPtr& sess, beast::flat_buffer& request);
  void actionPlay(const SessionPtr& sess, beast::flat_buffer& request);
  void actionSpectate(const SessionPtr& sess, beast::flat_buffer& request);
  void actionMove(const SessionPtr& sess, beast::flat_buffer& request);
  void actionEject(const SessionPtr& sess, beast::flat_buffer& request);
  void actionSplit(const SessionPtr& sess, beast::flat_buffer& request);
  void actionChatMessage(const SessionPtr& sess, beast::flat_buffer& request);
  void actionWatch(const SessionPtr& sess, beast::flat_buffer& request);

  void statistic();

private:
  using MessageHandler = std::function<void(const SessionPtr& sess, beast::flat_buffer& ms)>;
  using MessageHandlers = std::map<uint8_t, MessageHandler>;

  const MessageHandlers m_handlers {
    {IncomingPacket::Type::Ping,        std::bind(&Application::actionPing, this, _1, _2)},
    {IncomingPacket::Type::Greeting,    std::bind(&Application::actionGreeting, this, _1, _2)},
    {IncomingPacket::Type::Play,        std::bind(&Application::actionPlay, this, _1, _2)},
    {IncomingPacket::Type::Spectate,    std::bind(&Application::actionSpectate, this, _1, _2)},
    {IncomingPacket::Type::Move,        std::bind(&Application::actionMove, this, _1, _2)},
    {IncomingPacket::Type::Eject,       std::bind(&Application::actionEject, this, _1, _2)},
    {IncomingPacket::Type::Split,       std::bind(&Application::actionSplit, this, _1, _2)},
    {IncomingPacket::Type::ChatMessage, std::bind(&Application::actionChatMessage, this, _1, _2)},
    {IncomingPacket::Type::Watch,       std::bind(&Application::actionWatch, this, _1, _2)},
  };

  mutable std::mutex            m_mutex;
  asio::io_context              m_ioContext;
  asio::ip::tcp::socket         m_influxdb {m_ioContext};
  MySQLConnectionPool           m_mysqlConnectionPool;
  Sessions                      m_sessions;
  UsersCache                    m_users {m_mysqlConnectionPool};
  RoomManager                   m_roomManager;
  IOThreadPool                  m_ioThreadPool {"IO worker", m_ioContext};
  std::vector<std::thread>      m_threads;
  std::string                   m_configFileName;
  config::Config                m_config;
  Timer                         m_statisticTimer;
  std::size_t                   m_maxSessions {0};
  uint32_t                      m_registrations {0};
  ListenerPtr                   m_listener;
};

#endif /* THEGAME_APPLICATION_HPP */
