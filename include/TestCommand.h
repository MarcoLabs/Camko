#pragma once

#include "Command.h"
#include "CommandError.h"
#include <string>

class TestCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;
};