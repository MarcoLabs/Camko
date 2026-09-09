#pragma once

#include "Command.h"
#include "marco/toml/Toml.h"

class RunCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static std::expected<std::string, CommandError> GetProjectName(const Marco::Toml& toml);
};