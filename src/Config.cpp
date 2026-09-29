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

std::expected<ProjectConfig, CommandError> Config::ParseProjectConfig(const Marco::Toml& toml)
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

std::expected<BuildConfig, CommandError> Config::ParseBuildConfig(const Marco::Toml& toml)
{
	BuildConfig config{};

	auto buildSettings = toml["build"];
	if (! buildSettings || ! buildSettings->get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the build table in config.toml"});
	}

	auto buildSharedLibs = buildSettings->get()["build-shared-libs"];
	if (buildSharedLibs && buildSharedLibs->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The build-shared-libs option in the build table must be a boolean"});
	}

	config.buildSharedLibs = buildSharedLibs->get().AsBool().value();

	auto enableWarnings = buildSettings->get()["enable-warnings"];
	if (enableWarnings && enableWarnings->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The enable-warnings option in the build table must be a boolean"});
	}

	config.enableWarnings = enableWarnings->get().AsBool().value();

	auto warningsAsErrors = buildSettings->get()["warnings-as-errors"];
	if (warningsAsErrors && warningsAsErrors->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The warnings-as-errors option in the build table must be a boolean"});
	}

	config.warningsAsErrors = warningsAsErrors->get().AsBool().value();

	auto enableSanitizers = buildSettings->get()["enable-sanitizers"];
	if (enableSanitizers && enableSanitizers->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The enable-sanitizers option in the build table must be a boolean"});
	}

	config.enableSanitizers = enableSanitizers->get().AsBool().value();

	auto enableLto = buildSettings->get()["enable-lto"];
	if (enableLto && enableLto->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The enable-lto option in the build table must be a boolean"});
	}

	config.enableLto = enableLto->get().AsBool().value();

	auto enableCCache = buildSettings->get()["enable-ccache"];
	if (enableCCache && enableCCache->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The enable-ccache option in the build table must be a boolean"});
	}

	config.enableCcache = enableCCache->get().AsBool().value();

	return config;
}

std::expected<TestsConfig, CommandError> Config::ParseTestsConfig(const Marco::Toml& toml)
{

}

std::expected<std::vector<DependencyConfig>, CommandError> Config::ParseDependenciesConfig(const Marco::Toml& toml)
{

}
