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

	Artifacts artifacts{};
	
	auto& targets = toml["targets"];

	for (auto& [_, target] : targets.AsObject()->get())
	{
		auto type = target["type"];
		if (! type || ! type->get().IsString())
		{
			return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this. Error: with type not existing or not being a string"});
		}

		auto path = target["path"];
		if (! path || ! path->get().IsString())
		{
			return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this. Error: with path not existing or not being a string"});
		}

		if (type->get().AsString()->get() == "executable")
		{
			if (! artifacts.mainExecutable.empty())
			{
				return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this. Error: more than one main executable file is not allowed"});
			}

			artifacts.mainExecutable = std::filesystem::path(path->get().AsString()->get());
		}
		else if (type->get().AsString()->get() == "test")
		{
			if (! artifacts.testsExecutable.empty())
			{
				return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this. Error: more than one tests executable file is not allowed"});
			}

			artifacts.testsExecutable = std::filesystem::path(path->get().AsString()->get());
		}
		else if (type->get().AsString()->get() == "example")
		{
			artifacts.examplesExecutables.push_back(std::filesystem::path(path->get().AsString()->get()));
		}
		else
		{
			return std::unexpected(CommandError{false, "Unexpected error in the artifacts. You shouldn't be seeing this. Error: type is malformed"});
		}
	}

	return artifacts;
}
