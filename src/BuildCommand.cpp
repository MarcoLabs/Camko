#include "BuildCommand.h"
#include "Utils.h"
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <marco/toml/Toml.h>
#include <marco/toml/TomlReader.h>

CommandError BuildCommand::Execute(const std::vector<std::string>& args)
{
	auto result = utils::GetCamkoProjectRootDirectory();
	if (! result.has_value())
	{
		return result.error();
	}

	std::filesystem::path projectRoot = result.value();

	std::ifstream tomlFile(projectRoot / "config.toml");
	if (! tomlFile.is_open())
	{
		return CommandError{false, "Could not find config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	auto cMakeFileContents = ConstructCMakeLists(toml);
	if (! cMakeFileContents.has_value())
	{
		return cMakeFileContents.error();
	}

	std::filesystem::path camkoFolderPath = projectRoot / ".camko";
	
	std::ofstream cMakeListsFile(camkoFolderPath / "CMakeLists.txt");

	cMakeListsFile << *cMakeFileContents;
	
	cMakeListsFile.close();

	return CommandError{true, "No errors occured"};
}

std::string BuildCommand::Name() const
{
	return "build";
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeLists(const Marco::Toml& toml)
{
	std::string fileContents{};

	auto projectName = GetProjectName(toml);
	if (! projectName)
	{
		return std::unexpected(projectName.error());
	}

	auto partOfCmake = ConstructCMakeProjectDefinition(toml, *projectName);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;

	partOfCmake = ConstructCMakeLanguageStandard(toml);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;
	
	return fileContents;
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeProjectDefinition(const Marco::Toml& toml, const std::string& projectName)
{
	std::string partOfCmake{};

	auto projectSettings = toml["project"];
	if (! projectSettings || ! (*projectSettings).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}
	
	auto version = (*projectSettings).get()["version"];
	if (! version || ! version.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The version variable in config.toml was not set"});
	}

	partOfCmake += "cmake_minimum_required(VERSION 3.20)\n\n";

	partOfCmake += std::format("project({}\n", projectName);
	partOfCmake += std::format("\tVERSION {}\n", version.value().get().AsString()->get());

	auto description = (*projectSettings).get()["description"];
	if (description)
	{
		if (! description.value().get().IsString())
		{
			return std::unexpected(CommandError{false, "The description variable in config.toml must be a string"});
		}
		
		partOfCmake += std::format("DESCRIPTION {}\n", description.value().get().AsString()->get());
	}

	partOfCmake += "LANGUAGES CXX\n)\n\n";

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeLanguageStandard (const Marco::Toml& toml)
{
	std::string partOfCmake{};
	auto buildOptions = toml["build"];
	if (! buildOptions || ! (*buildOptions).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the build table in config.toml"});
	}

	auto cppVersion = (*buildOptions).get()["cpp-version"];
	if (! cppVersion || ! (*cppVersion).get().IsNumber())
	{
		return std::unexpected(CommandError{false, "Could not find the cpp-version option under the build table in config.toml"});
	}

	partOfCmake += std::format("set(CMAKE_CXX_STANDARD {})\n", (*cppVersion).get().AsNumber().value());
	partOfCmake += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
	partOfCmake += "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::GetProjectName(const Marco::Toml& toml)
{
	auto projectSettings = toml["project"];
	if (! projectSettings)
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}
	
	auto projectName = (*projectSettings).get()["name"];
	if (! projectName || ! projectName.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in config.toml not set or is not a string"});
	}

	return (*projectName).get().AsString()->get();
}