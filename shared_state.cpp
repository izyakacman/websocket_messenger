#include "shared_state.hpp"
#include "websocket_session.hpp"

#include <algorithm>

void SharedState::Join(WebsocketSession& session)
{
    sessions_[&session] = noname_;
}

void SharedState::Leave(WebsocketSession& session)
{
    sessions_.erase(&session);
}

void SharedState::SendAll(WebsocketSession* current_session, std::string message)
{
    if (sessions_[current_session] == noname_)
        return;

    for (auto session : sessions_)
    {
        auto const ss = std::make_shared<std::string const>(std::move(sessions_[current_session] + ": " + message));

        session.first->Send(ss);
    }
}

void SharedState::SendTo(WebsocketSession* current_session, std::string user_name, std::string message)
{
    if (sessions_[current_session] == noname_)
        return;

    auto session = GetSessionByName(user_name);

    if (session)
    {
        auto const ss = std::make_shared<std::string const>(std::move(sessions_[current_session] + ": " + message));
        session->Send(ss);
    }

    return;
}

void SharedState::SendToGroup(WebsocketSession* /*current_session*/, std::string /*group_name*/, std::string /*message*/)
{

}

void SharedState::AddUser(WebsocketSession* session, std::string user_name)
{
    if (GetSessionByName(user_name) == nullptr)
    {
        sessions_[session] = user_name;

        auto const ss = std::make_shared<std::string const>(std::move("Hello " + user_name));

        session->Send(ss);
    }
    else
    {
        auto const ss = std::make_shared<std::string const>(std::move("User " + user_name + " already registred"));

        session->Send(ss);
    }

    return;
}

WebsocketSession* SharedState::GetSessionByName(const std::string& user_name)
{
    auto itr = std::find_if(sessions_.cbegin(), sessions_.cend(), [&user_name](auto pair) {return pair.second == user_name;});

    return itr == sessions_.cend() ? nullptr : itr->first;
}