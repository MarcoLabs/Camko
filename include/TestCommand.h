#pragma once

#include "Command.h"
#include "CommandError.h"
#include <string>

class TestCommand : public Command
{
public:
	CommandError Execute(const Config& config, const std::vector<std::string>& args) override;
	std::string Name() const override;
};