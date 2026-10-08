#include "TestCommand.h"
#include "Artifacts.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils/General.h"
#include <format>
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

	auto artifacts = GetArtifactsByBuildType(config.buildConfig.type);
	if (! artifacts)
	{
		return artifacts.error();
	}

	std::cout << "\n\n";

	int errorCode = std::system(std::format("\"{}\"", artifacts->testsExecutable.string()).c_str());

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