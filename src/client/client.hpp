#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

/**
*   @brief Client session
*/
class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:

    ClientSession(net::io_context& ioc, std::ostream& output_stream);

    void Run(char const* host, char const* port);
    void Close();
    void Write(std::string msg);
    bool IsConnect();

private:

    void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
    void OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void OnHandshake(beast::error_code ec);
    void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
    void OnRead(beast::error_code ec, std::size_t bytes_transferred);
    void OnClose(beast::error_code ec);
    void Fail(beast::error_code ec, char const* what);

    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;
    std::ostream& output_stream_;
    bool connected_ = false;
};