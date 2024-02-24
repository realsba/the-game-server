// file   : src/Application.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Application.hpp"

#include "OutgoingPacket.hpp"
#include "AsioFormatter.hpp"
#include "HttpClient.hpp"
#include "ScopeExit.hpp"
#include "User.hpp"

#include "serialization.hpp"

#include <spdlog/spdlog.h>
#include <fmt/chrono.h>

#include <codecvt>

Application::Application(std::string configFileName)
  : m_configFileName(std::move(configFileName))
  , m_statisticTimer(m_ioContext.get_executor(), std::bind_front(&Application::statistic, this))
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

  m_config.load(m_configFileName);

  m_mysqlConnectionPool.init(m_config.mysql);

  if (m_config.influxdb.enabled) {
    m_statisticTimer.setInterval(m_config.influxdb.interval);
    m_statisticTimer.start();
  }

  m_listener->start();
  spdlog::info("Server started. address={}", m_config.server.address);

  m_roomManager.start(m_config.room);

  for (uint i = 0; i < m_config.server.numThreads; ++i) {
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

  m_influxdb.close();
  m_listener->stop();
  m_statisticTimer.stop();

  m_ioContext.stop();
  for (auto& thread : m_threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  m_threads.clear();
  m_ioContext.reset();
  m_roomManager.stop();

  spdlog::info("Server stopped");
}

void Application::save()
{
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
  auto user = sess->user();

  while (buffer.size()) {
    auto type = deserialize<uint8_t>(buffer);
    const auto& it = m_handlers.find(type);
    if (it == m_handlers.end()) {
      spdlog::warn("Received unknown message type: {}", type);
      return;
    }
    try {
      if (type != IncomingPacket::Ping) {
        sess->lastActivity(TimePoint::clock::now());
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
      const auto& user = sess->user();
      if (user) {
        userId = user->getId();
        auto* room = user->getRoom();
        if (room) {
          room->leave(sess);
        }
        sess->user(nullptr);
      }
      auto query = db->query("INSERT INTO `sessions` (userId,begin,end,ip) VALUES (%0,%1q,%2q,%3)");
      query.parse();
      query.execute(
        userId,
        fmt::to_string(sess->created()),
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
  OutgoingPacket::serializePong(*buffer);
  sess->send(buffer);
}

void Application::actionGreeting(const UserPtr& current, const SessionPtr& sess, beast::flat_buffer& request)
{
  if (current) {
    return; // user already logged in
  }

  const auto& buffer = std::make_shared<Buffer>();
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
    OutgoingPacket::serializeGreeting(*buffer, "");
  } else {
    user = m_users.create(sess->getRemoteEndpoint().address().to_v4().to_ulong());
    OutgoingPacket::serializeGreeting(*buffer, user->getSessId());
    ++m_registrations;
  }
  user->setSession(sess);
  sess->user(user);
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
  static const uint NAME_MAX_LENGTH = 16;

  auto name = deserialize<std::string>(request);
  auto color = deserialize<uint8_t>(request);
  if (user) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    const auto& wstr = cv.from_bytes(name);
    if (wstr.empty()) {
      name = "Player " + std::to_string(user->getId());
    } else if (wstr.length() > NAME_MAX_LENGTH) {
      name = cv.to_bytes(wstr.substr(0, NAME_MAX_LENGTH));
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

void Application::actionMove(const UserPtr& user, const SessionPtr& sess, beast::flat_buffer& request)
{
  Vec2D point;
  point.x = deserialize<int16_t>(request);
  point.y = deserialize<int16_t>(request);
  if (user) {
    auto* room = user->getRoom();
    if (room) {
      room->move(sess, point);
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

void Application::statistic()
{
  std::lock_guard<std::mutex> lock(m_mutex);

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
    Request request;
    request.version(11);
    request.method(http::verb::post);
    request.target(m_config.influxdb.path);
    request.set(http::field::host, m_config.influxdb.host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set("Content-Type", "text/plain; charset=utf-8");
    request.set("Accept", "application/json");
    request.set("Authorization", "Token " + m_config.influxdb.token);
    request.body() = data;
    request.prepare_payload();

    std::make_shared<HttpClient>(m_ioContext)->send(m_config.influxdb.host, m_config.influxdb.port, std::move(request));
  }
  m_maxSessions = m_sessions.size();
  m_registrations = 0;
}
