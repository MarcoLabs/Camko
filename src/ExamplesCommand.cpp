#include "ExamplesCommand.h"
#include "Artifacts.h"
#include "CommandRegistry.h"
#include "Utils/General.h"
#include <filesystem>
#include <iostream>

static bool IsExecutable(const std::filesystem::perms& permissions);

CommandError ExamplesCommand::Execute(const Config& config, const std::vector<std::string>& args)
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("build")->Execute(config, args);

	if (! error.valid)
	{
		return error;
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (! projectRoot)
	{
		return projectRoot.error();
	}

	auto artifacts = GetArtifactsByBuildType(config.buildConfig.type);
	if (! artifacts)
	{
		return artifacts.error();
	}

	error = RunAllExecutables(artifacts->examplesExecutables);

	return error;
}

std::string ExamplesCommand::Name() const
{
	return "examples";
}

CommandError ExamplesCommand::RunAllExecutables(const std::vector<std::filesystem::path>& examples)
{
	for (const auto& executable : examples)
	{
		std::cout << std::endl;

#ifdef _WIN32
		if (executable.extension() != ".exe")
		{
			continue;
		}
#else
		auto permissions = std::filesystem::status(executable).permissions();
		if (! IsExecutable(permissions))
		{
			continue;
		}
#endif

		std::cout << "\033[32mRunning " << executable.filename().string() << " \033[0m" << std::endl;

		std::string command = "\"" + executable.string() + "\"";

		int errorCode = std::system(command.c_str());

		if (errorCode != 0)
		{
			return CommandError{false, "Errors occured"};
		}
	}

	return CommandError{true, "No Errors occured"};
}

bool IsExecutable(const std::filesystem::perms& permissions)
{
	return  (permissions & std::filesystem::perms::owner_exec)  != std::filesystem::perms::none ||
			(permissions & std::filesystem::perms::group_exec)  != std::filesystem::perms::none ||
			(permissions & std::filesystem::perms::others_exec) != std::filesystem::perms::none;
}
