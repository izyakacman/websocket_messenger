#pragma once

#include <memory>
#include <string>
#include <unordered_map>

// Forward declaration
class WebsocketSession;

// Represents the shared server state
class SharedState
{

public:

    SharedState() = default;

    void Join(WebsocketSession& session);
    void Leave(WebsocketSession& session);
    void SendAll(WebsocketSession* current_session, std::string message);
    void SendTo(WebsocketSession* current_session, std::string user_name, std::string message);

    void SetUserName(WebsocketSession* session, std::string user_name);

private:

    WebsocketSession* GetSessionByName(const std::string& user_name);

    // This simple method of tracking
    // sessions only works with an implicit
    // strand (i.e. a single-threaded server)
    //std::unordered_set<websocket_session*> sessions_;
    std::unordered_map<WebsocketSession*, std::string> sessions_;

    const char* noname_ = "noname";
};
