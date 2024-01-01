// file   : Session.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_SESSION_HPP
#define THEGAME_SESSION_HPP

#include "SessionFwd.hpp"
#include "UserFwd.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <queue>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

using tcp = boost::asio::ip::tcp;

class Player;

// TODO: remove
using Buffer = std::vector<char>;
using BufferPtr = std::shared_ptr<Buffer>;

// TODO: забезпечити потокобезпечне використання user, player, observable
struct ConnectionData {
  asio::ip::address address;
//  SystemTimePoint create {SystemTimePoint::clock::now()};
//  TimePoint lastActivity {TimePoint::clock::now()};
  UserSPtr user;
  Player* player {nullptr};
  Player* observable {nullptr};
};

class Session : public std::enable_shared_from_this<Session>
{
public:
  ConnectionData connectionData {}; // TODO: implement

  using MessageHandler = std::function<void(const SessionPtr& sess, beast::flat_buffer& buffer)>;
  using OpenHandler = std::function<void(const SessionPtr& sess)>;
  using CloseHandler = std::function<void(const SessionPtr& sess)>;

  explicit Session(tcp::socket&& socket);

  void setMessageHandler(MessageHandler&& handler);
  void setOpenHandler(OpenHandler&& handler);
  void setCloseHandler(CloseHandler&& handler);
  void run();
  void send(const BufferPtr& buffer);

private:
  void doRun();
  void doSend(const BufferPtr& buffer);
  void doRead();
  void doWrite();

  void onAccept(beast::error_code ec);
  void onRead(beast::error_code ec, std::size_t bytesTransferred);
  void onWrite(beast::error_code ec, std::size_t bytesTransferred);

private:
  using SendQueue = std::queue<BufferPtr>;

  websocket::stream<beast::tcp_stream>  m_socket;
  beast::flat_buffer                    m_buffer;
  MessageHandler                        m_messageHandler;
  OpenHandler                           m_openHandler;
  CloseHandler                          m_closeHandler;
  SendQueue                             m_sendQueue;
};

#endif /* THEGAME_SESSION_HPP */
