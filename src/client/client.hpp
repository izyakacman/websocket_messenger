#pragma once

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{

public:

    // Resolver and socket require an io_context
    session(net::io_context& ioc, std::ostream& output_stream);

    // Start the asynchronous operation
    void run(char const* host, char const* port, char const* text);

    void on_resolve( beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);

private:

    // Report a failure
    void fail(beast::error_code ec, char const* what);

    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;
    std::ostream& output_stream_;
};