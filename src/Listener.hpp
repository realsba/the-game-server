// file   : src/Listener.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_LISTENER_HPP
#define THEGAME_LISTENER_HPP

#include "SessionFwd.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;

using tcp = boost::asio::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener> {
public:
  using AcceptHandler = std::function<void(const SessionPtr&)>;

  Listener(asio::io_context& ioc, tcp::endpoint&& endpoint, AcceptHandler&& handler);

  void start();
  void stop();

private:
  void doRun();
  void doStop();
  void doAccept();
  void onAccept(beast::error_code ec, tcp::socket socket);

private:
  mutable asio::io_context::strand      m_strand;
  asio::io_context&                     m_ioContext;
  tcp::endpoint                         m_endpoint;
  tcp::acceptor                         m_acceptor;
  AcceptHandler                         m_acceptHandler;
};

#endif /* THEGAME_LISTENER_HPP */
