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
		return CommandError{false, "Too many arguments"};
	}
	
	if (! args.empty())
	{
		projectRootDir += args[0];

		if (std::filesystem::exists(projectRootDir))
		{
			return CommandError{false, std::format("Folder {} already exists", projectRootDir)};
		}
	}
	else
	{
		projectRootDir = ".";
	}

	std::filesystem::path projectPath{};

	projectPath /= projectRootDir;

	CommandError result = InitializeEmptyProject(projectPath);
	if (! result.valid)
	{
		return result;
	}

	result = FillConfigAndMainFile(projectPath);
	if (! result.valid)
	{
		return result;
	}

	std::println("Successfully initialized new project under {}", std::filesystem::canonical(projectPath).string());

	return CommandError{true, "No errors occurred"};
}

std::string InitCommand::Name() const
{
	return "init";
}

CommandError InitCommand::InitializeEmptyProject(const std::filesystem::path& projectPath)
{
	bool result = std::filesystem::create_directories(projectPath / ".camko");

	if (! result)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	result = std::filesystem::create_directories(projectPath / "include");

	if (! result)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	result = std::filesystem::create_directories(projectPath / "src");

	if (! result)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	return CommandError{true, "No errors occurred"};
}

CommandError InitCommand::FillConfigAndMainFile(const std::filesystem::path& projectPath)
{
	std::ofstream configTomlFile(projectPath / "config.toml");
	if (! configTomlFile)
	{		
		return CommandError{false, "Could not create config.toml file"};
	}

	configTomlFile << defaults::kDefaultConfigToml;
	configTomlFile.close();

	std::ofstream mainFile(projectPath / "src/main.cpp");
	if (! mainFile)
	{		
		return CommandError{false, "Could not create config.toml file"};
	}

	mainFile << defaults::kDefaultMainCpp;
	mainFile.close();

	return CommandError{true, "No errors occurred"};
}