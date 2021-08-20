#include "precompiled.hpp"
#include "shared_state.hpp"
#include "websocket_session.hpp"

using boost::property_tree::ptree;

SharedState::SharedState()
{
    try
    {
        read_json(json_file_name_, groups_);
    }
    catch(boost::property_tree::json_parser_error&){} // json file not exist - no problem
}

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
        SendMsg(session.first, sessions_[current_session] + ": " + message);
    }
}

void SharedState::SendTo(WebsocketSession* current_session, std::string user_name, std::string message)
{
    if (sessions_[current_session] == noname_)
        return;

    auto session = GetSessionByName(user_name);

    if (session)
    {
        SendMsg(session, sessions_[current_session] + ": " + message);
        SendMsg(current_session, sessions_[current_session] + ": " + message);
    }

    return;
}

void SharedState::SendToGroup(WebsocketSession* /*current_session*/, std::string /*group_name*/, std::string /*message*/)
{

}

//---------------------------------------------------------------------------------------------------------------------------

void SharedState::AddUser(WebsocketSession* current_session, std::string user_name)
{
    if (GetSessionByName(user_name) == nullptr)
    {
        if (user_name.empty())
        {
            SendMsg(current_session, "Wrong user name!");
            return;
        }

        sessions_[current_session] = user_name;

        SendMsg(current_session, "Hello " + user_name);
    }
    else
    {
        SendMsg(current_session, "User " + user_name + " already registred");
    }

    return;
}

void SharedState::AddGroup(WebsocketSession* current_session, std::string group_name)
{
    auto user_name = sessions_.at(current_session);

    if(user_name == noname_)
    {
        SendMsgYouAreNotRegistered(current_session);
        return;
    }

    try 
    {
        auto node = groups_.get_child(group_name);

        SendMsg(current_session, "Group " + group_name + " already exist");
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        ptree group_node;
        group_node.push_back(std::make_pair(admin_, ptree(user_name)));
        groups_.put_child( group_name, group_node );
        write_json(json_file_name_, groups_);

        SendMsg(current_session, "Group " + group_name + " is created");
    }

}

void SharedState::AddUserToGroup(WebsocketSession* current_session, std::string group_name, std::string user_name)
{
    // TODO check duplicate

    auto current_user_name = sessions_.at(current_session);

    if(current_user_name == noname_)
    {
        SendMsgYouAreNotRegistered(current_session);
        return;
    }

    try
    {
        auto& group_node = groups_.get_child(group_name);
        const auto& admin_name = group_node.find(admin_)->second.data();

        if (current_user_name != admin_name)
        {
            SendMsgYouAreNotAdmin(current_session);
            return;
        }

        group_node.push_back(std::make_pair("users", ptree(user_name)));
        write_json(json_file_name_, groups_);

        SendMsg(current_session, "User " + user_name + " added into group " + group_name);
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        SendMsgGroupNotExist(current_session, group_name);
    }
}

//---------------------------------------------------------------------------------------------------------------------------

void SharedState::GetGroupsList(WebsocketSession* current_session)
{
    if(groups_.empty())
    {
        SendMsg(current_session, "No group added yet");
    }
    else
    {
        SendMsg(current_session, "Groups list:");

        for(const auto& group : groups_)
            SendMsg(current_session, group.first.data());
    }
}

void SharedState::GetGroupUsers(WebsocketSession* current_session, std::string group_name)
{
    try
    {
        auto& group_node = groups_.get_child(group_name);

        SendMsg(current_session, "Group " + group_name + " has next users");

        for(const auto& user : group_node)
            SendMsg(current_session, user.second.data() + ((user.first == admin_) ? " - admin" : ""));
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        SendMsgGroupNotExist(current_session, group_name);
    }
}

//---------------------------------------------------------------------------------------------------------------------------

void SharedState::DelUserFromGroup(WebsocketSession* current_session, std::string group_name, std::string user_name)
{
    try
    {
        auto& group_node = groups_.get_child(group_name);
        auto itr = FindUserIntoGroup(group_node, user_name);

        if(itr != group_node.end())
        {
            group_node.erase(itr);
            SendMsg(current_session, "User " + user_name + " was deleted from group " + group_name);
            write_json(json_file_name_, groups_);
        }
        else
        {
            SendMsg(current_session, "User " + user_name + " is not a member of the group " + group_name);
        }
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        SendMsgGroupNotExist(current_session, group_name);
    }
}

//---------------------------------------------------------------------------------------------------------------------------

WebsocketSession* SharedState::GetSessionByName(const std::string& user_name)
{
    auto itr = std::find_if(sessions_.cbegin(), sessions_.cend(), [&user_name](auto pair) {return pair.second == user_name;});

    return itr == sessions_.cend() ? nullptr : itr->first;
}

boost::property_tree::ptree::iterator
SharedState::FindUserIntoGroup(ptree& group_node, std::string user_name)
{
    return std::find_if(group_node.begin(), group_node.end(), [&user_name](const auto& pair)
    {
        return (pair.second.data() == user_name) ? true : false;
    });
}

void SharedState::SendMsg(WebsocketSession* session, std::string msg)
{
    auto const ss = std::make_shared<std::string const>(std::move(msg));

    session->Send(ss);
}

void SharedState::SendMsgGroupNotExist(WebsocketSession* session, std::string group_name)
{
    SendMsg(session, "Group " + group_name + " is no exist");
}

void SharedState::SendMsgYouAreNotRegistered(WebsocketSession* session)
{
    SendMsg(session, "You are not registered");
}

void SharedState::SendMsgYouAreNotAdmin(WebsocketSession* session)
{
    SendMsg(session, "You are not administrator of this group");
}