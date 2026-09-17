#include "RunCommand.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils.h"
#include "marco/toml/Toml.h"
#include "marco/toml/TomlReader.h"
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>

CommandError RunCommand::Execute(const std::vector<std::string>& args)
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("build")->Execute(args);

	if (!error.valid)
	{
		return error;
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (!projectRoot)
	{
		return projectRoot.error();
	}

	std::ifstream tomlFile(*projectRoot / "config.toml");
	if (!tomlFile.is_open())
	{
		return CommandError{false, "Could not find the config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();
	
	auto projectName = GetProjectName(toml);
	if (!projectName)
	{
		return projectName.error();
	}

	std::filesystem::path executablePath = *projectRoot / ".camko" / "build" / *projectName;
	std::string runArguments = ConstructRunArguments(args);

	std::cout << "\n\n";

	std::string command = std::format("{} {}", executablePath.string(), runArguments);
	
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