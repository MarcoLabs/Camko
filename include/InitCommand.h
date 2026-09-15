#pragma once

#include "Command.h"
#include "CommandError.h"
#include <filesystem>

class InitCommand : public Command
{
public:
	CommandError Execute(const std::vector<std::string>& args) override;
	std::string Name() const override;

private:
	static CommandError InitializeEmptyProject(const std::filesystem::path& projectPath);
	static CommandError FillConfigAndMainFile(const std::filesystem::path& projectPath);
	static CommandError AddGitIgnoreFile(const std::filesystem::path& projectPath);
};