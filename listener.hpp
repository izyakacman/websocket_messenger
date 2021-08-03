#pragma once

#include "net.hpp"
#include "commands_processor.hpp"

#include <memory>
#include <string>

// Forward declaration
class shared_state;

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener>
{

public:

    Listener( net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SharedState> const& state );

    // Start accepting incoming connections
    void Run();

private:

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::shared_ptr<SharedState> state_;
    std::shared_ptr<CommandsProcessor> cmd_processor_;

    void Fail( error_code ec, char const* what );
    void OnAccept( error_code ec );
};
