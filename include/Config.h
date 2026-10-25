#pragma once

#include "CommandError.h"
#include "marco/toml/Toml.h"
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

enum class BuildType
{
	Debug,
	Release,
	RelWithDebInfo,
	MinSizeRel,
	Invalid
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

	std::string buildSystem      = "Ninja";
	bool        buildSharedLibs  = false;
	bool        enableWarnings   = true;
	bool        warningsAsErrors = false;
	bool        enableSanitizers = false;
	bool        enableLto        = false;
	bool        enableCcache     = true;
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

struct ExtrasConfig
{
	bool preserveCache = false;
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

	TestsConfig                    testsConfig;
	ExamplesConfig                 examplesConfig;
	std::vector<DependencyConfig>  dependenciesConfig;
	ExtrasConfig                   extrasConfig;

	CommandError Parse(const std::filesystem::path& configFilepath);
	CommandError Parse(std::string_view tomlString);
	CommandError Parse(const Marco::Toml& toml);

	CommandError FillWithFlags(const std::vector<std::string> commandFlags);

	static std::string BuildTypeToString(BuildType type);
	static BuildType   StringToBuildType(const std::string& buildTypeString);

private:
	Config() = default;

	static std::expected<ProjectConfig,                 CommandError> ParseProjectConfig(const Marco::Toml& toml);
	static std::expected<BuildConfig,                   CommandError> ParseBuildConfig  (const Marco::Toml& toml);
	static std::expected<TestsConfig,                   CommandError> ParseTestsConfig(const Marco::Toml& toml);
	static std::expected<std::vector<DependencyConfig>, CommandError> ParseDependenciesConfig(const Marco::Toml& toml);
	static std::expected<ExamplesConfig,                CommandError> ParseExamplesConfig(const Marco::Toml& toml);
	static std::expected<ExtrasConfig,                  CommandError> ParseExtrasConfig(const Marco::Toml& toml);
};
