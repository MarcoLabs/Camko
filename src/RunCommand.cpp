#include "RunCommand.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Utils.h"
#include "marco/toml/Toml.h"
#include "marco/toml/TomlReader.h"
#include <expected>
#include <filesystem>
#include <fstream>
#include <print>


CommandError RunCommand::Execute(const std::vector<std::string>& args)
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

	std::filesystem::path executablePath = *projectRoot / ".camko" / "build" / *projectName;
	std::string runArguments = ConstructRunArguments(args);

	std::println("\n\n");

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

std::expected<std::string, CommandError> RunCommand::GetProjectName(const Marco::Toml& toml)
{
	auto projectSettings = toml["project"];
	if (! projectSettings)
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}

	auto projectName = (*projectSettings).get()["name"];
	if (! projectName || ! projectName.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in config.toml was not set or is not a string"});
	}

	return (*projectName).get().AsString()->get();
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