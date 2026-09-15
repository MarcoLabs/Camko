#pragma once

#include "Command.h"
#include <string>
#include <vector>

class ExampleCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static CommandError RunAllExecutables(const std::string& examplesPathDir);
};