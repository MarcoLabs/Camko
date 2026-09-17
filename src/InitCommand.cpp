#include "InitCommand.h"
#include "CommandError.h"
#include "Defaults.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <system_error>

static std::optional<std::ofstream> CreateFile(const std::filesystem::path& filePath);

CommandError InitCommand::Execute(const std::vector<std::string>& args)
{
	std::string projectRootDir{};

	if (args.size() > 1)
	{
		return CommandError{false, "Too many arguments"};
	}

	if (!args.empty())
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
	if (!result.valid)
	{
		return result;
	}

	result = FillConfigAndMainFile(projectPath);
	if (!result.valid)
	{
		return result;
	}

	result = AddGitIgnoreFile(projectPath);
	if (!result.valid)
	{
		return result;
	}

	std::cout << "Successfully initialized new project under "
			  << std::filesystem::canonical(projectPath).string() << std::endl;

	return CommandError{true, "No errors occurred"};
}

std::string InitCommand::Name() const
{
	return "init";
}

CommandError InitCommand::InitializeEmptyProject(const std::filesystem::path& projectPath)
{
	std::error_code ec{};
	
	bool result = std::filesystem::create_directories(projectPath / ".camko", ec);

	if (!result && ec.value() != 0)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	result = std::filesystem::create_directories(projectPath / "include", ec);

	if (!result && ec.value() != 0)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	result = std::filesystem::create_directories(projectPath / "src", ec);

	if (!result && ec.value() != 0)
	{
		return CommandError{false, "Could not initialize new project"};
	}

	return CommandError{true, "No errors occurred"};
}

CommandError InitCommand::FillConfigAndMainFile(const std::filesystem::path& projectPath)
{
	auto configTomlFile = CreateFile(projectPath / "config.toml");
	if (configTomlFile)
	{
		if (!configTomlFile.value().is_open())
		{
			return CommandError{false, "Could not create config.toml file"};
		}
		
		*configTomlFile << defaults::kDefaultConfigToml;
		(*configTomlFile).close();
	}

	auto mainFile = CreateFile(projectPath / "src/main.cpp");
	if (mainFile)
	{
		if (!mainFile.value().is_open())
		{
			return CommandError{false, "Could not create src/main.cpp file"};
		}
		
		*mainFile << defaults::kDefaultMainCpp;
		(*mainFile).close();
	}

	return CommandError{true, "No errors occurred"};
}

CommandError InitCommand::AddGitIgnoreFile(const std::filesystem::path& projectPath)
{
	std::ofstream gitIgnoreFile(projectPath / ".gitignore");
	if (!gitIgnoreFile)
	{
		return CommandError{false, "Could not create .gitignore file"};
	}

	gitIgnoreFile << defaults::kDefaultGitIgnore;
	gitIgnoreFile.close();

	return CommandError{true, "No errors occured"};
}

std::optional<std::ofstream> CreateFile(const std::filesystem::path& filePath)
{
	if (std::filesystem::exists(filePath))
	{
		return {};
	}

	return std::ofstream(filePath);
}