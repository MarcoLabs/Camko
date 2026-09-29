#pragma once

#include "Command.h"

class RunCommand : public Command
{
public:
	CommandError Execute(const Config& config, const std::vector<std::string>& args) override;
	std::string Name() const override;

private:	
	static std::string ConstructRunArguments(const std::vector<std::string>& args);
};