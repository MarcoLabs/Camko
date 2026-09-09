#pragma once

#include "Command.h"
#include "marco/toml/TomlReader.h"
#include <expected>
#include <string>
#include <vector>

class BuildCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static std::expected<std::string, CommandError> ConstructCMakeProjectDefinition(const Marco::TomlReader& reader);
};