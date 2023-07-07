// file   : WebsocketServer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "MemoryStream.hpp"
#include "TimePoint.hpp"
#include "UserFwd.hpp"
#include "Logger.hpp"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class Player;

struct WebsocketppConfig : public websocketpp::config::asio {
  struct ConnectionData {
    boost::asio::ip::address address;
    SystemTimePoint create {SystemTimePoint::clock::now()};
    TimePoint lastActivity {TimePoint::clock::now()};
    UserSPtr user; // TODO: забезпечити потокобезпечне використання user, player, observable
    Player* player {nullptr};
    Player* observable {nullptr};
  };

  typedef ConnectionData connection_base;
};

class WebsocketServer : public websocketpp::server<WebsocketppConfig> {
public:
  void send(websocketpp::connection_hdl hdl, const MemoryStream& ms)
  {
    if (hdl.expired()) {
      return;
    }
    websocketpp::lib::error_code ec;
    const auto& conn = get_con_from_hdl(hdl, ec);
    if (!conn || conn->get_state() != websocketpp::session::state::open) {
      return;
    }
    auto length = ms.length();
    if (length == 0) {
      LOG_WARN << "Output packet zero length";
    }
    m_totalSend += length;
    ++m_totalCnt;
    websocketpp::server<WebsocketppConfig>::send(hdl, ms.data(), length, websocketpp::frame::opcode::binary, ec);
    if (ec) {
      LOG_WARN << ec.message();
    }
  }

protected:
  size_t m_totalCnt {0};
  size_t m_totalSend {0};
};

#endif /* WEBSOCKET_SERVER_HPP */
