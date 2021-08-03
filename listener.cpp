#include "listener.hpp"
#include "websocket_session.hpp"

#include <iostream>

Listener::Listener( net::io_context& ioc, tcp::endpoint endpoint,  std::shared_ptr<SharedState> const& state)
    : acceptor_{ ioc }
    , socket_{ ioc }
    , state_{ state }
    , cmd_processor_{ std::make_shared<CommandsProcessor>(state) }
{
    error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        Fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true));
    if(ec)
    {
        Fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        Fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        Fail(ec, "listen");
        return;
    }
}

void Listener::Run()
{
    // Start accepting a connection
    acceptor_.async_accept(socket_,
        [self = shared_from_this()](error_code ec)
        {
            self->OnAccept(ec);
        });
}

// Report a failure
void Listener::Fail(error_code ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void Listener::OnAccept(error_code ec)
{
    if(ec)
        return Fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<WebsocketSession>(std::move(socket_), state_, cmd_processor_)->Run();

    // Accept another connection
    acceptor_.async_accept(socket_,
        [self = shared_from_this()](error_code ec)
        {
            self->OnAccept(ec);
        });
}
