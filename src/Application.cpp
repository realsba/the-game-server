// file   : Application.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Application.hpp"

#include "packet/InputPacketTypes.hpp"
#include "packet/OutputPacketTypes.hpp"
#include "packet/EmptyPacket.hpp"
#include "packet/PacketGreeting.hpp"

#include "MemoryStream.hpp"
#include "ScopeExit.hpp"
#include "TSRoom.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "User.hpp"
#include "util.hpp"

#include <sys/syscall.h>
#include <locale>
#include <codecvt>
#include <utility>

namespace ph = std::placeholders;
using namespace boost::asio;

Application::Application(std::string configFileName) :
  m_configFileName(std::move(configFileName)),
  m_timer(m_ioService, std::bind(&Application::update, this))
{
  // контейнер m_handlers не захищений мютексом, після заповнення не модифікується
  m_handlers.emplace(InputPacketTypes::Ping, std::bind(&Application::actionPing, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Greeting, std::bind(&Application::actionGreeting, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Play, std::bind(&Application::actionPlay, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Spectate, std::bind(&Application::actionSpectate, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Pointer, std::bind(&Application::actionPointer, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Eject, std::bind(&Application::actionEject, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Split, std::bind(&Application::actionSplit, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::ChatMessage, std::bind(&Application::actionChatMessage, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Watch, std::bind(&Application::actionWatch, this, ph::_1, ph::_2, ph::_3));
  m_handlers.emplace(InputPacketTypes::Paint, std::bind(&Application::actionPaint, this, ph::_1, ph::_2, ph::_3));
}

void Application::start()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_started) {
    return;
  }

  m_started = true;

  if (!m_config.loadFromFile(m_configFileName)) {
    throw std::runtime_error("Can't load config: \"" + m_configFileName + "\"");
  }

  m_mysqlConnectionPool.init(m_config.mysql);
  m_influxdb.open(ip::udp::v4());
  m_influxdb.connect(ip::udp::endpoint(ip::address::from_string(m_config.influxdbServer), m_config.influxdbPort));

  m_websocketServer.clear_error_channels(websocketpp::log::elevel::all);
  m_websocketServer.clear_access_channels(websocketpp::log::alevel::all);
  m_websocketServer.set_message_handler(std::bind(&Application::messageHandler, this, ph::_1, ph::_2));
  m_websocketServer.set_open_handler(std::bind(&Application::openHandler, this, ph::_1));
  m_websocketServer.set_close_handler(std::bind(&Application::closeHandler, this, ph::_1));
  m_websocketServer.init_asio(&m_ioService);
  m_websocketServer.set_reuse_addr(true);
  m_websocketServer.set_listen_backlog(m_config.listenBacklog);
  m_websocketServer.listen(m_config.address);
  m_websocketServer.start_accept();

  m_timer.setInterval(m_config.updateInterval);
  m_timer.start();

  LOG_INFO << "Server started. address=" << m_config.address;

  m_roomManager.start(m_config.roomThreads, m_config.room);
  for (uint i = 0; i < m_config.ioServiceThreads; ++i) {
    m_threads.emplace_back(
      [this]()
      {
        long int pid = syscall(SYS_gettid);
        LOG_INFO << "Start \"IO worker\" (" << pid << ")";
        while (true) {
          try {
            m_ioService.run();
            break;
          } catch (const std::exception& e) {
            LOG_ERROR << e.what();
          }
        }
        LOG_INFO << "Stop \"IO worker\" (" << pid << ")";
      }
    );
  }
}

void Application::stop()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_started) {
    return;
  }

  m_started = false;

  m_influxdb.close();
  m_websocketServer.stop();
  m_timer.stop();

  m_ioService.stop();
  for (std::thread& thread : m_threads) {
    thread.join();
  }
  m_threads.clear();
  m_ioService.reset();
  m_roomManager.stop();

  LOG_INFO << "Server stopped.";
}

void Application::save()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  try {
    m_users.save();
  } catch (const std::exception& e) {
    LOG_WARN << e.what();
  }
}

void Application::info()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  LOG_INFO << "Websocket connections: " << m_connections.size();
  LOG_INFO << "MySQL connections: " << m_mysqlConnectionPool.size();
  LOG_INFO << "Rooms: " << m_roomManager.size();
}

void Application::openHandler(ConnectionHdl hdl)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_connections.emplace(hdl);
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  conn->address = conn->get_socket().remote_endpoint().address();
  auto count = m_connections.size();
  if (count > m_maxConnections) {
    m_maxConnections = count;
  }
}

void Application::closeHandler(ConnectionHdl hdl)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_connections.erase(hdl)) {
    try {
      mysqlpp::Connection::thread_start();
      ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
      mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
      uint32_t userId = 0;
      const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
      auto& user = conn->user;
      if (user) {
        userId = user->getId();
        TSRoom* room = user->getRoom();
        if (room) {
          room->leave(hdl);
        }
        user.reset();
      }
      auto query = db->query("INSERT INTO `sessions` (userId,begin,end,ip) VALUES (%0,%1q,%2q,%3)");
      query.parse();
      query.execute(
        userId,
        toString(conn->create),
        toString(SystemTimePoint::clock::now()),
        conn->address.to_v4().to_ulong()
      );
    } catch (const std::exception& e) {
      LOG_WARN << e.what();
    }
  }
}

void Application::messageHandler(ConnectionHdl hdl, WebsocketServer::message_ptr msg)
{
  MemoryStream ms(msg->get_payload()); // TODO: оптимізувати використання MemoryStream

  WebsocketServer::connection_ptr conn;
  UserSPtr user;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    conn = m_websocketServer.get_con_from_hdl(hdl);
    user = conn->user;
  }

  while (ms.availableForRead()) {
    auto type = ms.readUInt8();
    // m_handlers ніде не модифікується, тому читаємо дані без захисту мютексом
    const auto& it = m_handlers.find(type);
    if (it == m_handlers.end()) {
      // дана версія протоколу не дозволяє ідентифікувати межі пакетів
      // якщо отримано неіснуючий тип пакету припиняємо обробку всієї черги пакетів
      return;
    }
    try {
      if (type != InputPacketTypes::Ping) {
        conn->lastActivity = TimePoint::clock::now();
      }
      it->second(user, hdl, ms);
    } catch (const std::exception& e) {
      LOG_WARN << e.what();
    }
  }
}

void Application::actionPing(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  EmptyPacket packet(OutputPacketTypes::Pong);
  packet.format(ms);
  m_websocketServer.send(hdl, ms);
}

void Application::actionGreeting(const UserSPtr& current, const ConnectionHdl& hdl, MemoryStream& request)
{
  if (current) {
    return; // user already logged in
  }
  const auto& conn = m_websocketServer.get_con_from_hdl(hdl);
  PacketGreeting packetGreeting;
  auto sid = request.readString();
  auto user = m_users.getUserBySessId(sid);
  if (user) {
    const auto& prevHdl = user->getConnection();
    if (!prevHdl.expired()) {
      TSRoom* room = user->getRoom();
      if (room) {
        room->leave(prevHdl);
      }
      websocketpp::lib::error_code ec;
      m_websocketServer.close(prevHdl, websocketpp::close::status::normal, "New session detected", ec);
    }
  } else {
    user = m_users.create(conn->address.to_v4().to_ulong());
    packetGreeting.sid = user->getSessId();
    ++m_registrations;
  }
  user->setConnection(hdl);
  conn->user = user;
  MemoryStream ms; // TODO: оптимізувати використання MemoryStream
  packetGreeting.format(ms);
  TSRoom* room = user->getRoom();
  if (!room) {
    room = m_roomManager.obtain();
    user->setRoom(room);
  }
  m_websocketServer.send(hdl, ms);
  room->join(hdl);
}

void Application::actionPlay(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  auto name = request.readString();
  const auto& color = request.readUInt8();
  if (user) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    const auto& wstr = cv.from_bytes(name);
    if (wstr.empty()) {
      name = "Player " + std::to_string(user->getId());
    } else if (wstr.length() > m_nameMaxLength) {
      name = cv.to_bytes(wstr.substr(0, m_nameMaxLength));
    }
    TSRoom* room = user->getRoom();
    if (room) {
      room->play(hdl, name, color);
    }
  }
}

void Application::actionSpectate(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  auto targetId = request.readUInt32();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->spectate(hdl, targetId);
    }
  }
}

void Application::actionPointer(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  Vec2D point;
  point.x = request.readInt16();
  point.y = request.readInt16();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->pointer(hdl, point);
    }
  }
}

void Application::actionEject(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  Vec2D point;
  point.x = request.readInt16();
  point.y = request.readInt16();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->eject(hdl, point);
    }
  }
}

void Application::actionSplit(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  Vec2D point;
  point.x = request.readInt16();
  point.y = request.readInt16();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->split(hdl, point);
    }
  }
}

void Application::actionWatch(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  uint32_t playerId = request.readInt32();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->watch(hdl, playerId);
    }
  }
}

void Application::actionPaint(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  Vec2D point;
  point.x = request.readInt16();
  point.y = request.readInt16();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      room->paint(hdl, point);
    }
  }
}

void Application::actionChatMessage(const UserSPtr& user, const ConnectionHdl& hdl, MemoryStream& request)
{
  auto text = request.readString();
  if (user) {
    TSRoom* room = user->getRoom();
    if (room) {
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
      const auto& wstr = cv.from_bytes(text);
      if (wstr.empty()) {
        return;
      }
      if (wstr.length() > 128) {
        text = cv.to_bytes(wstr.substr(0, 128));
      }
      room->chatMessage(hdl, text);
    }
  }
}

void Application::update()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  statistic();
  checkConnections();
}

void Application::checkConnections()
{
  // TODO: при великій кількості з'єднань перебирати весь контейнер m_connections неефективно
  auto endTime = TimePoint::clock::now() - m_config.connectionTTL;
  for (const auto& it : m_connections) {
    try {
      const auto& conn = m_websocketServer.get_con_from_hdl(it);
      if (conn->lastActivity <= endTime) {
        m_websocketServer.close(it, websocketpp::close::status::normal, "Connection timed out");
      }
    } catch (const std::exception& e) {
      LOG_WARN << e.what();
    }
  }
}

void Application::statistic()
{
  using namespace std::chrono;
  auto now = time_point_cast<seconds>(SystemTimePoint::clock::now());
  if (now.time_since_epoch().count() %  m_config.statisticInterval.count()) {
    return;
  }
  std::stringstream ss;
  if (m_maxConnections > 0) {
    ss << "connections value=" << m_maxConnections << "\n";
  }
  if (m_registrations > 0) {
    ss << "registrations value=" << m_registrations << "\n";
  }
  auto rooms = m_roomManager.size();
  if (rooms > 0) {
    ss << "rooms value=" << rooms << "\n";
  }
  const auto& data = ss.str();
  if (!data.empty()) {
    try {
      m_influxdb.send(boost::asio::buffer(data));
    } catch (const std::exception& e) {
      LOG_WARN << e.what();
    }
  }
  m_maxConnections = m_connections.size();
  m_registrations = 0;
}
