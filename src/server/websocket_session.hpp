#pragma once

#include "net.hpp"
#include "beast.hpp"
#include "shared_state.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// Forward declaration
class SharedState;
class CommandsProcessor;

/** Represents an active WebSocket connection to the server
*/
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession>
{

public:
    WebsocketSession(tcp::socket socket, std::shared_ptr<SharedState> const& state, std::shared_ptr<CommandsProcessor> const& cmd_processor);

    ~WebsocketSession();

    void Run();

    // Send a message
    void Send(std::shared_ptr<std::string const> const& ss);

private:

    void Fail(error_code ec, char const* what);
    void OnAccept(error_code ec);
    void OnRead(error_code ec, std::size_t bytes_transferred);
    void OnWrite(error_code ec, std::size_t bytes_transferred);

    beast::flat_buffer buffer_;
    websocket::stream<tcp::socket> ws_;
    std::shared_ptr<SharedState> state_;
    std::shared_ptr<CommandsProcessor> cmd_processor_;
    std::vector<std::shared_ptr<std::string const>> queue_;
};
