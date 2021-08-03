#pragma once

#include <string>
#include <memory>

#include "shared_state.hpp"

class CommandsProcessor
{
public:

	explicit CommandsProcessor(std::shared_ptr<SharedState> state);
	~CommandsProcessor() = default;

	void ProcessCommand(std::string cmd, WebsocketSession* session);

protected:

	std::shared_ptr<SharedState> state_;
};