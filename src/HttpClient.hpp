// file   : src/HttpClient.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef THEGAME_HTTP_CLIENT_HPP
#define THEGAME_HTTP_CLIENT_HPP


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

using Request = http::request<http::string_body>;

class HttpClient : public std::enable_shared_from_this<HttpClient>
{
public:
  explicit HttpClient(asio::io_context& ioc);

  void send(const std::string& host, uint16_t port, Request&& request);

private:
  void onResolve(const beast::error_code& ec, tcp::resolver::results_type results);
  void onConnect(const beast::error_code& ec, tcp::resolver::results_type::endpoint_type);
  void onWrite(const beast::error_code& ec, std::size_t bytesTransferred);
  void onRead(const beast::error_code& ec, std::size_t bytesTransferred);

private:
  tcp::resolver m_resolver;
  beast::tcp_stream m_stream;
  beast::flat_buffer m_buffer;
  Request m_request;
  http::response<http::string_body> m_response;
};


#endif /* THEGAME_HTTP_CLIENT_HPP */
