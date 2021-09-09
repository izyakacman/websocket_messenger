#include "precompiled.hpp"
#include "client.hpp"

ClientSession::ClientSession(net::io_context& ioc, std::ostream& output_stream) :
    resolver_(net::make_strand(ioc)),
    ws_(net::make_strand(ioc)),
    output_stream_{output_stream}
{
}

void ClientSession::Run(char const* host, char const* port)
{
    // Save these for later
    host_ = host;

    // Look up the domain name
    resolver_.async_resolve(host, port, beast::bind_front_handler(&ClientSession::OnResolve, shared_from_this()));
}

void ClientSession::OnResolve( beast::error_code ec, tcp::resolver::results_type results)
{
    if(ec)
        return Fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(&ClientSession::OnConnect, shared_from_this()));
}

void ClientSession::OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return Fail(ec, "connect");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client");
        }));

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/", beast::bind_front_handler(&ClientSession::OnHandshake, shared_from_this()));
}

void ClientSession::OnHandshake(beast::error_code ec)
{
    if(ec)
        return Fail(ec, "handshake");

    output_stream_ << "connection opened\n";

    connected_ = true;

    // Read
    ws_.async_read(buffer_, beast::bind_front_handler(&ClientSession::OnRead, shared_from_this()));
    
    // Send the message
    //ws_.async_write(net::buffer(text_), beast::bind_front_handler(&session::on_write, shared_from_this()));
}

void ClientSession::OnWrite(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return Fail(ec, "write");
    
    // Read a message into our buffer
    //ws_.async_read(buffer_, beast::bind_front_handler(&ClientSession::OnRead, shared_from_this()));
}

void ClientSession::OnRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return Fail(ec, "read");

    // The make_printable() function helps print a ConstBufferSequence
    output_stream_ << beast::make_printable(buffer_.data()) << std::endl;
    buffer_.clear();

    // Read
    ws_.async_read(buffer_, beast::bind_front_handler(&ClientSession::OnRead, shared_from_this()));
}

void ClientSession::OnClose(beast::error_code ec)
{
    if(ec)
        return Fail(ec, "close");

    // If we get here then the connection is closed gracefully

    output_stream_ << "connection closed\n";

    connected_ = false;
}

void ClientSession::Fail(beast::error_code ec, char const* what)
{
    output_stream_ << what << ": " << ec.message() << "\n";
}

void ClientSession::Close()
{
    // Close the WebSocket connection
    ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&ClientSession::OnClose, shared_from_this()));
}

void ClientSession::Write(std::string msg)
{
    // Send the message
    ws_.async_write(net::buffer(msg), beast::bind_front_handler(&ClientSession::OnWrite, shared_from_this()));
}

bool ClientSession::IsConnect()
{
    return connected_;
}