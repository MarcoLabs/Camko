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
	bool             enableTests;
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

using DependencyConfig = std::variant<SmallDependencyConfig, RegularDependencyConfig>;

class Config
{
public:
	Config(Config&)           = delete;
	Config operator=(Config&) = delete;
	
	Config(Config&&)           = delete;
	Config operator=(Config&&) = delete;

	static Config& Instance();
	
	ProjectConfig projectConfig{};
	BuildConfig   buildConfig{};
	
	std::optional<TestsConfig> testsConfig;
	std::optional<std::vector<DependencyConfig>> dependenciesConfig;

	std::expected<Config, CommandError> Parse(const std::filesystem::path& configFilepath);
	std::expected<Config, CommandError> Parse(const Marco::Toml& toml);

private:
	Config();
};

