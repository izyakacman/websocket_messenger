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

		state_->SetUserName(session, user_name );
	}
	else if (cmd_name == "#to")
	{
		std::string user_name, msg;

		iss >> user_name;

		std::getline(iss, msg);

		state_->SendTo(session, user_name, msg);
	}
	else
	{
		state_->SendAll(session, cmd);
	}
}