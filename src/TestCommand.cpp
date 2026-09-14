#include "TestCommand.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils.h"
#include "marco/toml/TomlReader.h"
#include <fstream>
#include <print>
#include <string>
#include <vector>

CommandError TestCommand::Execute(const std::vector<std::string>& args)
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

	std::ifstream tomlFile(*projectRoot / "config.toml");
	if (! tomlFile.is_open())
	{
		return CommandError{false, "Could not find the config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	auto projectName = GetProjectName(toml);
	if (! projectName)
	{
		return projectName.error();
	}

	std::filesystem::path executablePath = *projectRoot / ".camko" / "build" / (*projectName + "_tests");

	std::println("\n\n");

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