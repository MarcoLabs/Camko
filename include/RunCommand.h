#pragma once

#include "Command.h"
#include "marco/toml/Toml.h"

class RunCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:	
	static std::string ConstructRunArguments(const std::vector<std::string>& args);
};