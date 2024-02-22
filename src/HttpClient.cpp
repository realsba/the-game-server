// file   : src/HttpClient.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "HttpClient.hpp"

#include <spdlog/spdlog.h>

HttpClient::HttpClient(asio::io_context& ioc)
  : m_resolver(asio::make_strand(ioc))
  , m_stream(asio::make_strand(ioc))
{
}

void HttpClient::send(const std::string& host, uint16_t port, Request&& request)
{
  m_request = std::move(request);
  m_resolver.async_resolve(
    host, std::to_string(port), beast::bind_front_handler(&HttpClient::onResolve, shared_from_this())
  );
}

void HttpClient::onResolve(const beast::error_code& ec, tcp::resolver::results_type results)
{
  if (ec) {
    spdlog::error("Error resolving address: {}", ec.message());
    return;
  }

  m_stream.expires_after(std::chrono::seconds(30));
  m_stream.async_connect(results, beast::bind_front_handler(&HttpClient::onConnect, shared_from_this()));
}

void HttpClient::onConnect(const beast::error_code& ec, tcp::resolver::results_type::endpoint_type)
{
  if (ec) {
    spdlog::error("Error connecting to server: {}", ec.message());
    return;
  }

  m_stream.expires_after(std::chrono::seconds(30));
  http::async_write(m_stream, m_request, beast::bind_front_handler(&HttpClient::onWrite, shared_from_this()));
}

void HttpClient::onWrite(const beast::error_code& ec, std::size_t bytesTransferred)
{
  boost::ignore_unused(bytesTransferred);

  if (ec) {
    spdlog::error("Error writing data to server: {}", ec.message());
    return;
  }

  http::async_read(m_stream, m_buffer, m_response, beast::bind_front_handler(&HttpClient::onRead, shared_from_this()));
}

void HttpClient::onRead(const beast::error_code& ec, std::size_t bytesTransferred)
{
  boost::ignore_unused(bytesTransferred);

  if (ec) {
    spdlog::error("Error reading data from server: {}", ec.message());
    return;
  }

  if (m_response.result() != http::status::ok) {
    spdlog::debug("HTTP response status code: {}, body: {}", m_response.result_int(), m_response.body());
  }

  beast::error_code err;
  m_stream.socket().shutdown(tcp::socket::shutdown_both, err);

  if (err && err != beast::errc::not_connected) {
    spdlog::error("Error shutting down connection: {}", ec.message());
  }
}
