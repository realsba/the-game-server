// file   : Listener.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Listener.hpp"

#include "Session.hpp"
#include "AsioFormatter.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

Listener::Listener(asio::io_context& ioc, tcp::endpoint&& endpoint, AcceptHandler&& handler)
  : m_strand(ioc)
  , m_ioc(ioc)
  , m_endpoint(std::move(endpoint))
  , m_acceptor(ioc)
  , m_acceptHandler(std::move(handler))
{
}

void Listener::run()
{
  asio::post(m_strand.wrap(std::bind_front(&Listener::doRun, shared_from_this())));
}

void Listener::stop()
{
  asio::post(m_strand.wrap(std::bind_front(&Listener::doStop, shared_from_this())));
}

void Listener::doRun()
{
  beast::error_code ec;

  m_acceptor.open(m_endpoint.protocol(), ec);
  if (ec) {
    throw std::runtime_error(fmt::format(
      "Failed to open listener socket with endpoint {}: {}", m_endpoint, ec.message()
    ));
  }

  m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
  if (ec) {
    throw std::runtime_error(fmt::format(
      "Failed to set option for listener socket with endpoint {}: {}", m_endpoint, ec.message()
    ));
  }

  m_acceptor.bind(m_endpoint, ec);
  if (ec) {
    throw std::runtime_error(fmt::format(
      "Failed to bind listener socket with endpoint {}: {}", m_endpoint, ec.message()
    ));
  }

  m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
  if (ec) {
    throw std::runtime_error(fmt::format(
      "Failed to listen on listener socket with endpoint {}: {}", m_endpoint, ec.message()
    ));
  }

  doAccept();
}

void Listener::doStop()
{
  m_acceptor.cancel();
  m_acceptor.close();
}

void Listener::doAccept()
{
  m_acceptor.async_accept(
    asio::make_strand(m_ioc), asio::bind_executor(m_strand, std::bind_front(&Listener::onAccept, shared_from_this()))
  );
}

void Listener::onAccept(beast::error_code ec, tcp::socket socket)
{
  if (ec) {
    spdlog::error("Acceptance failed: {}", ec.message());
  } else {
    m_acceptHandler(std::make_shared<Session>(std::move(socket)));
  }

  doAccept();
}
