#include "RunCommand.h"
#include "Artifacts.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils/General.h"
#include <expected>
#include <filesystem>
#include <iostream>

CommandError RunCommand::Execute(const Config& config, const std::vector<std::string>& args)
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

	auto artifacts = GetArtifactsByBuildType(config.buildConfig.type);
	if (! artifacts)
	{
		return artifacts.error();
	}
	
	std::string runArguments = ConstructRunArguments(args);

	std::cout << "\n\n";

	std::string command = std::format("\"{}\" {}", artifacts->mainExecutable.string(), runArguments);

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

std::string RunCommand::Name() const
{
	return "run";
}

std::string RunCommand::ConstructRunArguments(const std::vector<std::string>& args)
{
	std::string argsString{};

	for (size_t i = 0; i < args.size(); i++)
	{
		argsString += args[i];
		argsString.push_back(' ');
	}

	return argsString;
}
