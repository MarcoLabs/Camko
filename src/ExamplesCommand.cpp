#include "ExamplesCommand.h"
#include "CommandRegistry.h"
#include "Utils.h"
#include <filesystem>
#include <print>

static bool IsExecutable(const std::filesystem::perms& permissions);

CommandError ExamplesCommand::Execute(const std::vector<std::string>& args)
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("build")->Execute(args);

	if (! error.valid)
	{
		return error;
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (! projectRoot)
	{
		return projectRoot.error();
	}

	std::filesystem::path examplesPath = *projectRoot / ".camko" / "build" / "examples";

	error = RunAllExecutables(examplesPath.string());

	return error;
}

std::string ExamplesCommand::Name() const
{
	return "examples";
}

CommandError ExamplesCommand::RunAllExecutables(const std::string& examplesPathDir)
{
	for (const auto& executable : std::filesystem::directory_iterator(examplesPathDir))
	{
		std::println("\n");

		if (! executable.is_regular_file())
		{
			continue;
		}

		const std::filesystem::path& p = executable.path();

#ifdef _WIN32
		if (p.extension() != ".exe")
		{
			continue;
		}
#else
		auto permissions = std::filesystem::status(p).permissions();
		if (! IsExecutable(permissions))
		{
			continue;
		}
#endif

		std::println("\033[32mRunning {} \033[0m", p.filename().string());

		std::string command = "\"" + p.string() + "\"";

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
