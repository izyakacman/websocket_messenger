#include "precompiled.hpp"
#include "client.hpp"

session::session(net::io_context& ioc, std::ostream& output_stream) : 
    resolver_(net::make_strand(ioc)),
    ws_(net::make_strand(ioc)),
    output_stream_{output_stream}
{
}

void session::run(char const* host, char const* port)
{
    // Save these for later
    host_ = host;

    // Look up the domain name
    resolver_.async_resolve(host, port, beast::bind_front_handler(&session::on_resolve, shared_from_this()));
}

void session::on_resolve( beast::error_code ec, tcp::resolver::results_type results)
{
    if(ec)
        return fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(&session::on_connect, shared_from_this()));
}

void session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail(ec, "connect");

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
                    " websocket-client-async");
        }));

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/", beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}

void session::on_handshake(beast::error_code ec)
{
    if(ec)
        return fail(ec, "handshake");

    output_stream_ << "connection opened\n";
    
    // Send the message
    //ws_.async_write(net::buffer(text_), beast::bind_front_handler(&session::on_write, shared_from_this()));
}

void session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");
    
    // Read a message into our buffer
    ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "read");

    // The make_printable() function helps print a ConstBufferSequence
    output_stream_ << beast::make_printable(buffer_.data()) << std::endl;
    buffer_.clear();

    // Close the WebSocket connection
    //ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&session::on_close, shared_from_this()));
}

void session::on_close(beast::error_code ec)
{
    if(ec)
        return fail(ec, "close");

    // If we get here then the connection is closed gracefully

    output_stream_ << "connection closed\n";
}

void session::fail(beast::error_code ec, char const* what)
{
    output_stream_ << what << ": " << ec.message() << "\n";
}

void session::Close()
{
    // Close the WebSocket connection
    ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&session::on_close, shared_from_this()));
}

void session::Write(std::string msg)
{
    // Send the message
    ws_.async_write(net::buffer(msg), beast::bind_front_handler(&session::on_write, shared_from_this()));
}
