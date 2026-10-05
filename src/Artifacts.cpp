#include "Artifacts.h"
#include "CommandError.h"
#include "Config.h"
#include "Utils/General.h"
#include "marco/toml/Toml.h"
#include "marco/toml/TomlReader.h"
#include <expected>
#include <filesystem>
#include <fstream>


std::expected<Artifacts, CommandError> GetArtifactsByBuildType(BuildType type)
{
	if (type == BuildType::Invalid)
	{
		return std::unexpected(CommandError{false, "Unknown build type in the config file"});
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (! projectRoot)
	{
		return std::unexpected(CommandError{false, "Could not find the project root"});
	}

	std::filesystem::path artifactsDirectory = *projectRoot / ".camko" / "artifacts";

	std::ifstream artifactsTomlFile(artifactsDirectory / std::format("{}.toml", Config::BuildTypeToString(type)));
	if (! artifactsTomlFile.is_open())
	{
		return std::unexpected(CommandError{false, "Could not find the executables. Did you run `camko build` before running?"});
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(artifactsTomlFile);
	if (! toml["targets"].IsObject())
	{
		return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this."});
	}

	auto& targets = toml["targets"];

	for (auto& target : targets.AsObject()->get())
	{
		
	}
}
