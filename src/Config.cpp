#include "Config.h"
#include "marco/toml/TomlReader.h"
#include <fstream>

Config& Config::Instance()
{
	static Config config{};

	return config;
}

CommandError Config::Parse(const std::filesystem::path& configFilepath)
{
	std::ifstream tomlFile(configFilepath);
	if (! tomlFile.is_open())
	{
		return CommandError{false, "Could not find the config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	return Parse(toml);
}

CommandError Config::Parse(const Marco::Toml& toml)
{
	auto projectConfig = ParseProjectConfig(toml);
	if (! projectConfig)
	{
		return projectConfig.error();
	}

	this->projectConfig = *projectConfig;

	auto buildConfig = ParseBuildConfig(toml);
	if (! buildConfig)
	{
		return buildConfig.error();
	}

	this->buildConfig = *buildConfig;

	auto testsConfig = ParseTestsConfig(toml);
	if (! testsConfig)
	{
		return testsConfig.error();
	}

	this->testsConfig = *testsConfig;

	auto dependenciesConfig = ParseDependenciesConfig(toml);
	if (! dependenciesConfig)
	{
		return dependenciesConfig.error();
	}

	this->dependenciesConfig = *dependenciesConfig;

	return CommandError{true, "No errors occured while parsing config"};
}

std::expected<ProjectConfig, CommandError> ParseProjectConfig(const Marco::Toml& toml)
{
	ProjectConfig config{};
	
	auto projectSettings = toml["project"];
	if (! projectSettings || ! (*projectSettings).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}

	auto name = (*projectSettings).get()["name"];
	if (! name || ! name->get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in config.toml was not set or is not a string"});
	}

	config.name = name->get().AsString()->get();

	auto version = (*projectSettings).get()["version"];
	if (! version || ! version.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The version variable in config.toml was not set or is not a string"});
	}

	config.version = version->get().AsString()->get();

	auto description = (*projectSettings).get()["description"];
	if (description)
	{
		if (! description->get().IsString())
		{
			return std::unexpected(CommandError{false, "The description variable in config.toml must be a string"});
		}

		config.description = description->get().AsString()->get();
	}

	return config;
}

std::expected<BuildConfig, CommandError> ParseBuildConfig  (const Marco::Toml& toml)
{
	
}

std::expected<TestsConfig, CommandError> ParseTestsConfig(const Marco::Toml& toml)
{
	
}

std::expected<std::vector<DependencyConfig>, CommandError> ParseDependenciesConfig(const Marco::Toml& toml)
{
	
}
