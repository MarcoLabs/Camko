#include "Config.h"
#include "marco/toml/TomlReader.h"
#include <fstream>

Config& Config::Instance()
{
	static Config config{};

	return config;
}

std::expected<Config, CommandError> Config::Parse(const std::filesystem::path& configFilepath)
{
	std::ifstream tomlFile(configFilepath);
	if (! tomlFile.is_open())
	{
		return std::unexpected(CommandError{false, "Could not find the config.toml file"});
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	return Parse(toml);
}

std::expected<Config, CommandError> Config::Parse(const Marco::Toml& toml)
{
	
}
