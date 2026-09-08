#include "InitCommand.h"
#include "CommandError.h"
#include "Defaults.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <print>

CommandError InitCommand::Execute(const std::vector<std::string>& args)
{
	std::string projectRootDir{};

	if (args.size() > 1)
	{
		std::println("Too many arguments");
		
		return CommandError{false, "Too many arguments"};
	}
	
	if (! args.empty())
	{
		projectRootDir += args[0];
	}
	else
	{
		projectRootDir = ".";
	}

	std::filesystem::path projectPath{};

	projectPath /= projectRootDir;

	bool result = std::filesystem::create_directories(projectPath / ".camko");

	if (! result)
	{
		std::println("Could not initilize new project");

		return CommandError{false, "Could not initilize new project"};
	}

	std::ofstream configTomlFile(projectPath / "config.toml");
	if (! configTomlFile)
	{
		std::println("Could not create config.toml file");
		
		return CommandError{false, "Could not create config.toml file"};
	}

	configTomlFile << defaults::kDefaultConfigToml;
	configTomlFile.close();

	result = std::filesystem::create_directories(projectPath / "include");

	if (! result)
	{
		std::println("Could not initilize new project");

		return CommandError{false, "Could not initilize new project"};
	}

	result = std::filesystem::create_directories(projectPath / "src");

	if (! result)
	{
		std::println("Could not initilize new project");

		return CommandError{false, "Could not initilize new project"};
	}

	std::ofstream mainFile(projectPath / "src/main.cpp");
	if (! mainFile)
	{
		std::println("Could not create src/main.cpp file");
		
		return CommandError{false, "Could not create config.toml file"};
	}

	mainFile << defaults::kDefaultMainCpp;
	mainFile.close();

	std::println("Successfully initialized new project under {}", std::filesystem::canonical(projectPath).string());

	return CommandError{true, "No errors occured"};
}

std::string InitCommand::Name() const
{
	return "init";
}