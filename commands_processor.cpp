#include "commands_processor.hpp"

#include <iostream>
#include <sstream>

CommandsProcessor::CommandsProcessor(std::shared_ptr<SharedState> state) :
	state_{state}
{
}

void CommandsProcessor::ProcessCommand(std::string cmd, WebsocketSession* session)
{
	std::istringstream iss{cmd};

	std::string cmd_name;

	iss >> cmd_name;

	if (cmd_name == "#iam")
	{	
		std::string user_name;

		iss >> user_name;

		state_->AddUser(session, user_name );
	}
	else if (cmd_name == "#to")
	{
		std::string user_name, msg;

		iss >> user_name;

		std::getline(iss, msg);

		state_->SendTo(session, user_name, msg);
	}
	else if(cmd_name == "#to_group")
	{
		std::string group_name, msg;

		iss >> group_name;

		std::getline(iss, msg);

		state_->SendToGroup(session, group_name, msg);
		
	}
	else if(cmd_name == "#create_group")
	{
		std::string group_name;

		iss >> group_name;

		state_->AddGroup(session, group_name );
	}
	else if(cmd_name == "#add_user_to_group") // #add_user_to_group <group> <user>
	{
		std::string group_name;
		std::string user_name;
		iss >> group_name >> user_name;

		state_->AddUserToGroup(session, group_name, user_name );
	}
	else
	{
		state_->SendAll(session, cmd);
	}
}