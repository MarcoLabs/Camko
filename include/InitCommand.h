#pragma once

#include "Command.h"

class InitCommand : public Command
{
public:
	int Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;
};