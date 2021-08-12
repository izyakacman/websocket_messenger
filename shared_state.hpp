#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>

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
    void SendToGroup(WebsocketSession* current_session, std::string group_name, std::string message);
    void SendMsg(WebsocketSession* session, std::string msg);

    void AddUser(WebsocketSession* current_session, std::string user_name);
    void AddGroup(WebsocketSession* current_session, std::string group_name);
    void AddUserToGroup(WebsocketSession* current_session, std::string group_name, std::string user_name);

private:

    WebsocketSession* GetSessionByName(const std::string& user_name);

    // This simple method of tracking
    // sessions only works with an implicit
    // strand (i.e. a single-threaded server)
    //std::unordered_set<websocket_session*> sessions_;
    std::unordered_map<WebsocketSession*, std::string> sessions_;

    const char* noname_ = "noname";
    const char* json_file_name_ = "server.json";
    const char* json_user_name_ = ".user";
    boost::property_tree::ptree groups_;
};
