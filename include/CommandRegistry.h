#pragma once

#include "Command.h"
#include <memory>
#include <string>
#include <unordered_map>

class CommandRegistry
{
public:
	CommandRegistry(CommandRegistry&) = delete;
	CommandRegistry operator=(CommandRegistry&) = delete;

	CommandRegistry(CommandRegistry&&) = delete;
	CommandRegistry operator=(CommandRegistry&&) = delete;
	
	static CommandRegistry& Instance();

	Command* Find(const std::string& name) const;

private:
	CommandRegistry();

	std::unordered_map<std::string, std::unique_ptr<Command>> m_commands;
};