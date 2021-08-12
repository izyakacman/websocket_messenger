#include "shared_state.hpp"
#include "websocket_session.hpp"

#include <algorithm>

#include <boost/property_tree/json_parser.hpp>

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

void SharedState::AddUser(WebsocketSession* current_session, std::string user_name)
{
    if (GetSessionByName(user_name) == nullptr)
    {
        sessions_[current_session] = user_name;

        SendMsg(current_session, "Hello " + user_name);
    }
    else
    {
        SendMsg(current_session, "User " + user_name + " already registred");
    }

    return;
}

#include <iostream>

void SharedState::AddGroup(WebsocketSession* current_session, std::string group_name)
{
    try
    {
        auto node = groups_.get_child(group_name);

        SendMsg(current_session, "Group " + group_name + " already exist");
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        boost::property_tree::ptree empty_node;
        /*
        boost::property_tree::ptree user1_node;
        user1_node.put("", "user1");
        boost::property_tree::ptree user2_node;
        user2_node.put("", "user2");
        empty_node.push_back(std::make_pair("", user1_node));
        empty_node.push_back(std::make_pair("", user2_node));
        //
        groups_.put_child(group_name, empty_node);

        //
        auto gr = groups_.get_child( group_name);
        boost::property_tree::ptree user3_node;
        user3_node.put("", "user3");
        gr.push_back(std::make_pair("", user3_node));

        boost::property_tree::json_parser::write_json( std::cout, groups_, false );
        */

       //
       using boost::property_tree::ptree;
        groups_.put_child( group_name, ptree() );
        auto& array = groups_.get_child( group_name );
        array.push_back( std::make_pair( "", ptree("foo") ) );
        array.push_back( std::make_pair( "", ptree("bar") ) );
        boost::property_tree::json_parser::write_json( std::cout, groups_, false );
       //

        write_json(json_file_name_, groups_);
        /*
        groups_.put(group_name + json_user_name_, "");
        write_json(json_file_name_, groups_);
*/
        SendMsg(current_session, "Group " + group_name + " is created");
    }

}

void SharedState::AddUserToGroup(WebsocketSession* current_session, std::string group_name, std::string user_name)
{
    try
    {
        std::cout << "1111\n";
        auto group_node = groups_.get_child(group_name);
        //
        for(auto& user : group_node)
            std::cout << user.second.data() << std::endl;
        //
        // boost::property_tree::ptree user_node;
        // user_node.put("", user_name);
        group_node.push_back(std::make_pair("", boost::property_tree::ptree(user_name)));
        //groups_.add_child(group_name, group_node);

        //
        std::cout << "____________________\n";
        for(auto& user : group_node)
            std::cout << user.second.data() << std::endl;
        //


        write_json(json_file_name_, groups_);
        boost::property_tree::json_parser::write_json( std::cout, groups_, false );

        SendMsg(current_session, "User " + user_name + " added into group " + group_name);
    }
    catch(boost::property_tree::ptree_bad_path&)
    {
        SendMsg(current_session, "Group " + group_name + " is no exist");
    }
/*
    groups_.put(group_name + json_user_name_, user_name);
    write_json(json_file_name_, groups_);

    SendMsg(current_session, "User " + user_name + " added into group " + group_name);
*/
}

WebsocketSession* SharedState::GetSessionByName(const std::string& user_name)
{
    auto itr = std::find_if(sessions_.cbegin(), sessions_.cend(), [&user_name](auto pair) {return pair.second == user_name;});

    return itr == sessions_.cend() ? nullptr : itr->first;
}

void SharedState::SendMsg(WebsocketSession* session, std::string msg)
{
    auto const ss = std::make_shared<std::string const>(std::move(msg));

    session->Send(ss);
}