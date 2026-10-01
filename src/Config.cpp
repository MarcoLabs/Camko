#include "Config.h"
#include "CommandError.h"
#include "marco/toml/TomlReader.h"
#include <fstream>
#include <optional>

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
		return CommandError{false, "Could not find the camko config file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	return Parse(toml);
}

CommandError Config::Parse(std::string_view tomlString)
{
	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlString.data());

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
	
	auto examplesConfig = ParseExamplesConfig(toml);
	if (! examplesConfig)
	{
		return examplesConfig.error();
	}

	this->examplesConfig = *examplesConfig;

	return CommandError{true, "No errors occured while parsing config"};
}

std::string Config::BuildTypeToString(BuildType type)
{
	switch (type)
	{
		case BuildType::Debug:
			return "Debug";
		case BuildType::Release:
			return "Release";
		case BuildType::MinSizeRel:
			return "MinSizeRel";
		case BuildType::RelWithDebInfo:
			return "RelWithDebInfo";
		case BuildType::Invalid:
			return "Invalid";
	}
}

BuildType Config::StringToBuildType(const std::string& buildTypeString)
{
	if (buildTypeString == "Debug")
	{
		return BuildType::Debug;
	}
	else if (buildTypeString == "Release")
	{
		return BuildType::Release;
	}
	else if (buildTypeString == "MinSizeRel")
	{
		return BuildType::MinSizeRel;
	}
	else if (buildTypeString == "RelWithDebInfo")
	{
		return BuildType::RelWithDebInfo;
	}
	else
	{
		return BuildType::Invalid;
	}
}

std::expected<ProjectConfig, CommandError> Config::ParseProjectConfig(const Marco::Toml& toml)
{
	ProjectConfig config{};

	auto projectSettings = toml["project"];
	if (! projectSettings || ! (*projectSettings).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the project table in camko config"});
	}

	auto name = (*projectSettings).get()["name"];
	if (! name || ! name->get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in camko config was not set or is not a string"});
	}

	config.name = name->get().AsString()->get();

	auto version = (*projectSettings).get()["version"];
	if (! version || ! version.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The version variable in camko config was not set or is not a string"});
	}

	config.version = version->get().AsString()->get();

	auto description = (*projectSettings).get()["description"];
	if (description)
	{
		if (! description->get().IsString())
		{
			return std::unexpected(CommandError{false, "The description variable in camko config must be a string"});
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
		return std::unexpected(CommandError{false, "Could not find the build table in camko config"});
	}

	auto type = buildSettings->get()["type"];
	if (! type || ! type->get().IsString())
	{
		return std::unexpected(CommandError{false, "The build type in the build table is required and must be a string"});
	}

	config.type = Config::StringToBuildType(type->get().AsString()->get());

	auto cppVersion = buildSettings->get()["cpp-version"];
	if (! cppVersion || ! cppVersion->get().IsNumber())
	{
		return std::unexpected(CommandError{false, "The cpp-version in the build table is required and must be a nubmer"});
	}

	config.cppVersion = cppVersion->get().AsNumber().value();

	auto sourceDirectory = buildSettings->get()["source-directory"];
	if (! sourceDirectory || ! sourceDirectory->get().IsString())
	{
		return std::unexpected(CommandError{false, "The source-directory in the build table is required and must be a string"});
	}

	config.sourceDirectory = sourceDirectory->get().AsString()->get();

	auto headerDirectory = buildSettings->get()["header-directory"];
	if (! headerDirectory || ! headerDirectory->get().IsString())
	{
		return std::unexpected(CommandError{false, "The header-directory in the build table is required and must be a string"});
	}

	config.headerDirectory = headerDirectory->get().AsString()->get();

	auto buildSharedLibs = buildSettings->get()["build-shared-libs"];
	if (buildSharedLibs)
	{
		if (! buildSharedLibs->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The build-shared-libs option in the build table must be a boolean"});
		}

		config.buildSharedLibs = buildSharedLibs->get().AsBool().value();
	}

	auto enableWarnings = buildSettings->get()["enable-warnings"];
	if (enableWarnings)
	{
		if (! enableWarnings->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-warnings option in the build table must be a boolean"});
		}

		config.enableWarnings = enableWarnings->get().AsBool().value();
	}

	auto warningsAsErrors = buildSettings->get()["warnings-as-errors"];
	if (warningsAsErrors)
	{
		if (! warningsAsErrors->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The warnings-as-errors option in the build table must be a boolean"});
		}

		config.warningsAsErrors = warningsAsErrors->get().AsBool().value();
	}

	auto enableSanitizers = buildSettings->get()["enable-sanitizers"];
	if (enableSanitizers)
	{
		if (! enableSanitizers->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-sanitizers option in the build table must be a boolean"});
		}

		config.enableSanitizers = enableSanitizers->get().AsBool().value();
	}

	auto enableLto = buildSettings->get()["enable-lto"];
	if (enableLto)
	{
		if (! enableLto->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-lto option in the build table must be a boolean"});
		}

		config.enableLto = enableLto->get().AsBool().value();
	}

	auto enableCCache = buildSettings->get()["enable-ccache"];
	if (enableCCache)
	{
		if (! enableCCache->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-ccache option in the build table must be a boolean"});
		}

		config.enableCcache = enableCCache->get().AsBool().value();
	}

	return config;
}

std::expected<std::optional<TestsConfig>, CommandError> Config::ParseTestsConfig(const Marco::Toml& toml)
{
	auto testsSettings = toml["tests"];
	if (! testsSettings)
	{
		return std::nullopt;
	}
	else if (! testsSettings->get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the tests table in camko config. Is it a table?"});
	}

	TestsConfig config{};

	auto enableTests = testsSettings->get()["enable-tests"];
	if (! enableTests)
	{
		config.enableTests = false;
	}
	else
	{
		if (! enableTests->get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-tests option in the tests table must be a boolean"});
		}

		config.enableTests = enableTests->get().AsBool().value();
	}

	if (! config.enableTests)
	{
		return config;
	}

	auto testsDirectory = testsSettings->get()["tests-directory"];
	if (! testsDirectory || ! testsDirectory->get().IsString())
	{
		return std::unexpected(CommandError{false, "Could not find the tests-directory field in the tests table in camko config"});
	}

	config.testsDirectory = testsDirectory->get().AsString()->get();

	return config;
}

std::expected<std::optional<std::vector<DependencyConfig>>, CommandError> Config::ParseDependenciesConfig(const Marco::Toml& toml)
{
	auto dependenciesSettings = toml["dependencies"];
	if (! dependenciesSettings || ! dependenciesSettings->get().IsArray())
	{
		return std::nullopt;
	}

	std::vector<DependencyConfig> dependencies{};

	const Marco::TomlArray dependenciesArr = dependenciesSettings->get().AsArray().value().get();

	for (const auto& dependency : dependenciesArr)
	{
		auto libPackageName = dependency["find-package-name"];
		if (libPackageName && libPackageName->get().IsString())
		{
			dependencies.push_back(SmallDependencyConfig{libPackageName->get().AsString()->get()});

			continue;
		}

		RegularDependencyConfig config{};

		auto name = dependency["name"];
		if (! name || ! name->get().IsString())
		{
			return std::unexpected(CommandError{false, "The name in the dependencies array does not exist or isnt a string"});
		}

		config.name = name->get().AsString()->get();

		auto repo = dependency["repo"];
		if (! repo || ! repo->get().IsString())
		{
			return std::unexpected(CommandError{false, "The repo in the dependencies array does not exist or isnt a string"});
		}

		config.repo = repo->get().AsString()->get();

		auto version = dependency["version"];
		if (version)
		{
			if (! version->get().IsString())
			{
				return std::unexpected(CommandError{false, "The version in the dependencies array does not exist or isnt a string"});
			}

			config.version = version->get().AsString()->get();
		}


		auto linkTarget = dependency["link-target"];
		if (linkTarget)
		{
			if (! linkTarget->get().IsString())
			{
				return std::unexpected(CommandError{false, "The link-target in the dependencies array isnt a string"});
			}

			config.linkTarget = linkTarget->get().AsString()->get();
		}
		else
		{
			config.linkTarget = std::format("{0}::{0}", config.name);
		}

		dependencies.push_back(config);
	}

	return dependencies;
}

std::expected<std::optional<ExamplesConfig>,CommandError> Config::ParseExamplesConfig(const Marco::Toml& toml)
{
	auto examplesSettings = toml["examples"];
	if (! examplesSettings || ! examplesSettings->get().IsObject())
	{
		return std::nullopt;
	}

	auto enableExamples = examplesSettings->get()["enable-examples"];
	if (! enableExamples)
	{
		return std::nullopt;
	}
	else if (! enableExamples->get().IsBool())
	{
		return std::unexpected(CommandError{false, "The enable-examples in the examples table isnt a bool"});
	}

	ExamplesConfig config{};

	config.enableExamples = enableExamples->get().AsBool().value();

	if (config.enableExamples)
	{
		auto examplesDirectory = examplesSettings->get()["examples-directory"];
		if (! examplesDirectory || ! examplesDirectory->get().IsString())
		{
			return std::unexpected(CommandError{false, "Could not find the examples-directory field in the tests table in camko config"});
		}

		config.examplesDirectory = examplesDirectory->get().AsString()->get();
	}

	return config;
}
