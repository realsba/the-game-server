// file   : src/Session.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_SESSION_HPP
#define THEGAME_SESSION_HPP

#include "SessionFwd.hpp"
#include "PlayerFwd.hpp"
#include "UserFwd.hpp"

#include "TimePoint.hpp"
#include "types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <queue>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

using tcp = boost::asio::ip::tcp;

class Player;

class UserData {
public:
  SystemTimePoint created() const;
  TimePoint lastActivity() const;
  UserPtr user() const;
  PlayerPtr player() const;
  PlayerPtr observable() const;

  void lastActivity(const TimePoint& value);
  void user(const UserPtr& value);
  void player(PlayerPtr value);
  void observable(PlayerPtr value);

private:
  mutable std::mutex    m_mutex;
  const SystemTimePoint m_created {SystemTimePoint::clock::now()};  // thread-safe, const
  TimePoint             m_lastActivity {TimePoint::clock::now()};
  UserPtr               m_user {nullptr};
  PlayerPtr             m_player {nullptr};                         // thread-safe, accessed only from Room
  PlayerPtr             m_observable {nullptr};                     // thread-safe, accessed only from Room
};

class Session : public std::enable_shared_from_this<Session>, public UserData
{
public:
  using MessageHandler = std::function<void(const SessionPtr& sess, beast::flat_buffer& buffer)>;
  using OpenHandler = std::function<void(const SessionPtr& sess)>;
  using CloseHandler = std::function<void(const SessionPtr& sess)>;

  explicit Session(tcp::socket&& socket);

  tcp::endpoint getRemoteEndpoint() const;

  void setMessageHandler(MessageHandler&& handler);
  void setOpenHandler(OpenHandler&& handler);
  void setCloseHandler(CloseHandler&& handler);

  void run();
  void close();
  void send(const BufferPtr& buffer);

private:
  void doRun();
  void doClose();
  void doSend(const BufferPtr& buffer);
  void doRead();
  void doWrite();

  void onAccept(beast::error_code ec);
  void onClose(beast::error_code ec);
  void onRead(beast::error_code ec, std::size_t bytesTransferred);
  void onWrite(beast::error_code ec, std::size_t bytesTransferred);

private:
  using SendQueue = std::queue<BufferPtr>;

  websocket::stream<beast::tcp_stream>  m_socket;
  const tcp::endpoint                   m_remoteEndpoint;
  MessageHandler                        m_messageHandler;
  OpenHandler                           m_openHandler;
  CloseHandler                          m_closeHandler;
  beast::flat_buffer                    m_buffer {};
  SendQueue                             m_sendQueue {};
  bool                                  m_closed {false};
};

#endif /* THEGAME_SESSION_HPP */
