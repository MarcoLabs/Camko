#include "TestCommand.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils/General.h"
#include <iostream>
#include <string>
#include <vector>

CommandError TestCommand::Execute(const Config& config, const std::vector<std::string>& args)
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("build")->Execute(config, args);

	if (!error.valid)
	{
		return error;
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (!projectRoot)
	{
		return projectRoot.error();
	}

	std::filesystem::path executablePath = utils::GetBuildFolderPath(*projectRoot, config.buildConfig.buildSystem) / (config.projectConfig.name + "_tests");

	std::cout << "\n\n";

	std::string command = executablePath.string();

	int errorCode = std::system(command.c_str());

	if (errorCode == 0)
	{
		return CommandError{true, "No errors occured"};
	}
	else
	{
		return CommandError{false, "Errors occured"};
	}
}

std::string TestCommand::Name() const
{
	return "test";
}