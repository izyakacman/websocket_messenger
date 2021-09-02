#include "websocket_session.hpp"
#include "commands_processor.hpp"
#include "precompiled.hpp"

WebsocketSession::WebsocketSession(tcp::socket socket, std::shared_ptr<SharedState> const& state, std::shared_ptr<CommandsProcessor> const& cmd_processor)
    : ws_{ std::move(socket) }
    , state_{ state }
    , cmd_processor_{ cmd_processor }
{
}

WebsocketSession::~WebsocketSession()
{
    // Remove this session from the list of active sessions
    state_->Leave(*this);
}

void WebsocketSession::Fail(error_code ec, char const* what)
{
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void WebsocketSession::OnAccept(error_code ec)
{
    // Handle the error, if any
    if(ec)
        return Fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->Join(*this);

    // Read a message
    ws_.async_read(buffer_,
        [sp = shared_from_this()](error_code ec, std::size_t bytes)
        {
            sp->OnRead(ec, bytes);
        });
}

void WebsocketSession::OnRead(error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return Fail(ec, "read");

    // Process command
    cmd_processor_->ProcessCommand(beast::buffers_to_string(buffer_.data()), this);

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(buffer_,
        [sp = shared_from_this()](error_code ec, std::size_t bytes)
        {
            sp->OnRead(ec, bytes);
        });
}

void WebsocketSession::Send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(net::buffer(*queue_.front()),
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->OnWrite(ec, bytes);
        });
}

void WebsocketSession::OnWrite(error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return Fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(net::buffer(*queue_.front()),
            [sp = shared_from_this()](error_code ec, std::size_t bytes)
            {
                sp->OnWrite(ec, bytes);
            });
}

void WebsocketSession::Run()
{
    // Accept the websocket handshake
    ws_.async_accept(std::bind(&WebsocketSession::OnAccept, shared_from_this(), std::placeholders::_1));        
}
