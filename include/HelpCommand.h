#pragma once

#include "Command.h"
#include "CommandError.h"
#include <string>
#include <unordered_map>
#include <vector>

class HelpCommand : Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static std::unordered_map<std::string, std::string_view> helpMessages;
};