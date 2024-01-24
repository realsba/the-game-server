// file   : Application.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Application.hpp"

#include "packet/OutputPacketTypes.hpp"
#include "packet/EmptyPacket.hpp"
#include "packet/PacketGreeting.hpp"

#include "AsioFormatter.hpp"
#include "ScopeExit.hpp"
#include "Room.hpp"
#include "Player.hpp"
#include "User.hpp"
#include "util.hpp"

#include <spdlog/spdlog.h>
#include <fmt/chrono.h>

#include <locale>
#include <codecvt>
#include <utility>

Application::Application(std::string configFileName)
  : m_configFileName(std::move(configFileName))
  , m_timer(m_ioContext.get_executor(), std::bind_front(&Application::update, this))
{
  m_listener = std::make_shared<Listener>(
    m_ioContext,
    tcp::endpoint{asio::ip::make_address("0.0.0.0"), 3333},
    [this](const SessionPtr& sess)
    {
      sess->setMessageHandler(std::bind(&Application::sessionMessageHandler, this, _1, _2));
      sess->setOpenHandler(std::bind(&Application::sessionOpenHandler, this, _1));
      sess->setCloseHandler(std::bind(&Application::sessionCloseHandler, this, _1));
      sess->run();
    }
  );
}

void Application::start()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_started) {
    return;
  }

  m_started = true;

  m_config.load(m_configFileName);

  m_mysqlConnectionPool.init(m_config.mysql);
  m_influxdb.open(asio::ip::udp::v4());
  m_influxdb.connect(
    asio::ip::udp::endpoint(asio::ip::address::from_string(m_config.influxdbServer), m_config.influxdbPort)
  );

  m_listener->run();

  m_timer.setInterval(m_config.updateInterval);
  m_timer.start();

  spdlog::info("Server started. address={}", m_config.address);

  m_roomManager.start(m_config.roomThreads, m_config.room);
  for (uint i = 0; i < m_config.ioContextThreads; ++i) {
    m_threads.emplace_back(
      [this]
      {
        spdlog::info("Start \"IO worker\"");
        while (true) {
          try {
            m_ioContext.run();
            break;
          } catch (const std::exception& e) {
            spdlog::error("Application error: {}", e.what());
          }
        }
        spdlog::info("Stop \"IO worker\"");
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
  m_listener->stop();
  m_timer.stop();

  m_ioContext.stop();
  for (auto& thread : m_threads) {
    thread.join();
  }
  m_threads.clear();
  m_ioContext.reset();
  m_roomManager.stop();

  spdlog::info("Server stopped");
}

void Application::save()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  try {
    m_users.save();
  } catch (const std::exception& e) {
    spdlog::warn("An exception occurred while saving data: {}", e.what());
  }
}

void Application::info()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  spdlog::info("Websocket sessions: {}", m_sessions.size());
  spdlog::info("MySQL connections: {}", m_mysqlConnectionPool.size());
  spdlog::info("Rooms: {}", m_roomManager.size());
}

void Application::sessionMessageHandler(const SessionPtr& sess, beast::flat_buffer& buffer)
{
  auto user = sess->connectionData.user;

  while (buffer.size()) {
    auto type = deserialize<uint8_t>(buffer);
    const auto& it = m_handlers.find(type);
    if (it == m_handlers.end()) {
      spdlog::warn("Received unknown message type: {}", type);
      return;
    }
    try {
      if (type != InputPacketTypes::Ping) {
        sess->connectionData.lastActivity = TimePoint::clock::now();
      }
      it->second(user, sess, buffer);
    } catch (const std::exception& e) {
      spdlog::error("Exception caught while handling message: {}", e.what());
    }
  }
}

void Application::sessionOpenHandler(const SessionPtr& sess)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_sessions.emplace(sess);
  m_maxSessions = std::max(m_maxSessions, m_sessions.size());
}

void Application::sessionCloseHandler(const SessionPtr& sess)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_sessions.erase(sess)) {
    try {
      mysqlpp::Connection::thread_start();
      ScopeExit onExit([](){ mysqlpp::Connection::thread_end(); });
      mysqlpp::ScopedConnection db(m_mysqlConnectionPool, true);
      uint32_t userId = 0;
      auto& user = sess->connectionData.user;
      if (user) {
        userId = user->getId();
        auto* room = user->getRoom();
        if (room) {
          room->leave(sess);
        }
        user.reset();
      }
      auto query = db->query("INSERT INTO `sessions` (userId,begin,end,ip) VALUES (%0,%1q,%2q,%3)");
      query.parse();
      query.execute(
        userId,
        fmt::to_string(sess->connectionData.created),
        fmt::to_string(SystemTimePoint::clock::now()),
        sess->getRemoteEndpoint().address().to_v4().to_ulong()
      );
    } catch (const std::exception& e) {
      spdlog::warn("An exception occurred while saving data: {}", e.what());
    }
  }
}

void Application::actionPing(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  const auto& buffer = std::make_shared<Buffer>();
  EmptyPacket packet(OutputPacketTypes::Pong);
  packet.format(*buffer);
  sess->send(buffer);
}

void Application::actionGreeting(const UserPtr& current, const SessionPtr& sess, beast::flat_buffer& request)
{
  if (current) {
    return; // user already logged in
  }
  PacketGreeting packetGreeting;
  auto sid = deserialize<std::string>(request);
  auto user = m_users.getUserBySessId(sid);
  if (user) {
    const auto& prevSession = user->getSession();
    if (prevSession) {
      auto* room = user->getRoom();
      if (room) {
        room->leave(prevSession);
      }
      prevSession->close();
    }
  } else {
    user = m_users.create(sess->getRemoteEndpoint().address().to_v4().to_ulong());
    packetGreeting.sid = user->getSessId();
    ++m_registrations;
  }
  user->setSession(sess);
  sess->connectionData.user = user;
  const auto& buffer = std::make_shared<Buffer>();
  packetGreeting.format(*buffer);
  auto* room = user->getRoom();
  if (!room) {
    room = m_roomManager.obtain();
    user->setRoom(room);
  }
  sess->send(buffer);
  room->join(sess);
}

void Application::actionPlay(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  auto name = deserialize<std::string>(request);
  auto color = deserialize<uint8_t>(request);
  if (user) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    const auto& wstr = cv.from_bytes(name);
    if (wstr.empty()) {
      name = "Player " + std::to_string(user->getId());
    } else if (wstr.length() > m_nameMaxLength) {
      name = cv.to_bytes(wstr.substr(0, m_nameMaxLength));
    }
    auto* room = user->getRoom();
    if (room) {
      room->play(sess, name, color);
    }
  }
}

void Application::actionSpectate(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  auto targetId = deserialize<uint32_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->spectate(sess, targetId);
    }
  }
}

void Application::actionPoint(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  Vec2D point;
  point.x = deserialize<int16_t>(request);
  point.y = deserialize<int16_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->point(sess, point);
    }
  }
}

void Application::actionEject(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  Vec2D point;
  point.x = deserialize<int16_t>(request);
  point.y = deserialize<int16_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->eject(sess, point);
    }
  }
}

void Application::actionSplit(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  Vec2D point;
  point.x = deserialize<int16_t>(request);
  point.y = deserialize<int16_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->split(sess, point);
    }
  }
}

void Application::actionWatch(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  auto playerId = deserialize<uint32_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->watch(sess, playerId);
    }
  }
}

void Application::actionChatMessage(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  auto text = deserialize<std::string>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
      const auto& wstr = cv.from_bytes(text);
      if (wstr.empty()) {
        return;
      }
      if (wstr.length() > 128) {
        text = cv.to_bytes(wstr.substr(0, 128));
      }
      room->chatMessage(sess, text);
    }
  }
}

void Application::update()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  statistic();
}

void Application::statistic()
{
  using namespace std::chrono;
  auto now = time_point_cast<seconds>(SystemTimePoint::clock::now());
  if (now.time_since_epoch().count() %  m_config.statisticInterval.count()) {
    return;
  }
  std::stringstream ss;
  if (m_maxSessions > 0) {
    ss << "connections value=" << m_maxSessions << "\n";
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
      m_influxdb.send(asio::buffer(data));
    } catch (const std::exception& e) {
      spdlog::warn("Exception caught while sending data to InfluxDB: {}", e.what());
    }
  }
  m_maxSessions = m_sessions.size();
  m_registrations = 0;
}
