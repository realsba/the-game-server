// file   : Session.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Session.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/dispatch.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace http = beast::http;

Session::Session(tcp::socket&& socket)
  : m_socket(std::move(socket))
  , m_remoteEndpoint([&]() -> tcp::endpoint{
      try {
        return tcp::endpoint(m_socket.next_layer().socket().remote_endpoint());
      } catch (...) {
        return {};
      }
    }())
{
}

tcp::endpoint Session::getRemoteEndpoint() const
{
  return m_remoteEndpoint;
}

void Session::setMessageHandler(MessageHandler&& handler)
{
  m_messageHandler = std::move(handler);
}

void Session::setOpenHandler(OpenHandler&& handler)
{
  m_openHandler = std::move(handler);
}

void Session::setCloseHandler(CloseHandler&& handler)
{
  m_closeHandler = std::move(handler);
}

void Session::run()
{
  asio::dispatch(m_socket.get_executor(), std::bind_front(&Session::doRun, shared_from_this()));
}

void Session::send(const BufferPtr& buffer)
{
  asio::dispatch(m_socket.get_executor(), std::bind_front(&Session::doSend, shared_from_this(), buffer));
}

void Session::doRun()
{
  m_socket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
  m_socket.set_option(websocket::stream_base::decorator(
    [](websocket::response_type& res)
    {
      res.set(http::field::server, "TheGame server");
    }
  ));
  m_socket.async_accept(
    asio::bind_executor(m_socket.get_executor(), std::bind_front(&Session::onAccept, shared_from_this()))
  );
}

void Session::doSend(const BufferPtr& buffer)
{
  m_sendQueue.push(buffer);
  if (m_sendQueue.size() == 1) {
    asio::dispatch(m_socket.get_executor(), std::bind_front(&Session::doWrite, shared_from_this()));
  }
}

void Session::doRead()
{
  m_socket.async_read(
    m_buffer, asio::bind_executor(m_socket.get_executor(), std::bind_front(&Session::onRead, shared_from_this()))
  );
}

void Session::doWrite()
{
  const auto& data = m_sendQueue.front();
  auto buffer = asio::buffer(data->data(), data->size());
  m_socket.async_write(
    buffer, asio::bind_executor(m_socket.get_executor(), std::bind_front(&Session::onWrite, shared_from_this()))
  );
}

void Session::onAccept(beast::error_code ec)
{
  if (ec) {
    spdlog::error("accept: {}", ec.message());
    return;
  }

  m_socket.binary(true);

  if (m_openHandler) {
    m_openHandler(shared_from_this());
  }

  doRead();
}

void Session::onRead(beast::error_code ec, std::size_t bytesTransferred)
{
  boost::ignore_unused(bytesTransferred);

  if (ec) {
    spdlog::error("Failed to read: {}", ec.message());
    if (m_closeHandler) {
      m_closeHandler(shared_from_this());
    }
    return;
  }

  if (m_messageHandler) {
    m_messageHandler(shared_from_this(), m_buffer);
  }

  m_buffer.consume(bytesTransferred);
  doRead();
}

void Session::onWrite(beast::error_code ec, std::size_t bytesTransferred)
{
  if (ec) {
    spdlog::error("Failed to write: {}", ec.message());
  }

  m_sendQueue.pop();

  if (!m_sendQueue.empty()) {
    asio::dispatch(m_socket.get_executor(), std::bind_front(&Session::doWrite, shared_from_this()));
  }
}
