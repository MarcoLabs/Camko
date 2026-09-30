#pragma once

#include "CommandError.h"
#include "marco/toml/Toml.h"
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class BuildType
{
	Debug,
	Release,
	RelWithDebInfo,
	MinSizeRel
};

struct ProjectConfig
{
	std::string name;
	std::string version;
	std::string description;
};

struct BuildConfig
{
	BuildType   type                  = BuildType::Debug;
	int         cppVersion            = 23;
	std::string sourceDirectory       = "src";
	std::string headerDirectory       = "include";
	bool             buildSharedLibs  = false;
	bool             enableWarnings   = true;
	bool             warningsAsErrors = false;
	bool             enableSanitizers = false;
	bool             enableLto        = false;
	bool             enableCcache     = true;
};

struct TestsConfig
{
	bool        enableTests = false;
	std::string testsDirectory;
};

struct SmallDependencyConfig
{
	std::string findPackageName;
};

struct RegularDependencyConfig
{
	std::string name;
	std::string repo;
	std::string version;
	std::string linkTarget;
};

struct ExamplesConfig
{
	bool        enableExamples    = false;
	std::string examplesDirectory;
};

using DependencyConfig = std::variant<SmallDependencyConfig, RegularDependencyConfig>;

class Config
{
public:
	Config(const Config&)           = delete;
	Config operator=(const Config&) = delete;
	
	Config(Config&&)           = delete;
	Config operator=(Config&&) = delete;

	static Config& Instance();
	
	ProjectConfig projectConfig{};
	BuildConfig   buildConfig{};
	
	std::optional<TestsConfig>                   testsConfig;
	std::optional<std::vector<DependencyConfig>> dependenciesConfig;
	std::optional<ExamplesConfig>                examplesConfig;

	CommandError Parse(const std::filesystem::path& configFilepath);
	CommandError Parse(const Marco::Toml& toml);
	
	static std::string BuildTypeToString(BuildType type);

private:
	Config() = default;

	static std::expected<ProjectConfig,                                CommandError> ParseProjectConfig(const Marco::Toml& toml);
	static std::expected<BuildConfig,                                  CommandError> ParseBuildConfig  (const Marco::Toml& toml);
	static std::expected<std::optional<TestsConfig>,                   CommandError> ParseTestsConfig(const Marco::Toml& toml);
	static std::expected<std::optional<std::vector<DependencyConfig>>, CommandError> ParseDependenciesConfig(const Marco::Toml& toml);
	static std::expected<std::optional<ExamplesConfig>,                CommandError> ParseExamplesConfig(const Marco::Toml& toml);
};
