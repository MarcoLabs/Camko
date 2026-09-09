#include "BuildCommand.h"
#include "Utils.h"
#include <filesystem>
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

	std::ifstream tomlFile(projectRoot.string() + "config.toml");
	if (! tomlFile.is_open())
	{
		return CommandError{false, "Could not find config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	std::string cMakeFileContents{};

	
}

std::string BuildCommand::Name() const
{
	return "build";
}
