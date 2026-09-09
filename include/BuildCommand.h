#pragma once

#include "Command.h"
#include "marco/toml/Toml.h"
#include <expected>
#include <string>
#include <vector>

class BuildCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static std::expected<std::string, CommandError> ConstructCMakeLists             (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructCMakeProjectDefinition (const Marco::Toml& toml, const std::string& projectName);
	static std::expected<std::string, CommandError> ConstructCMakeLanguageStandard  (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructUserConfigurableOptions(const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructTestOptions            (const Marco::Toml& toml);
	
	static std::expected<std::string, CommandError> GetProjectName(const Marco::Toml& toml);
};