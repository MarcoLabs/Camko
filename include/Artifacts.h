#pragma once

#include "CommandError.h"
#include "Config.h"
#include <expected>
#include <filesystem>
#include <vector>

struct Artifacts
{
	std::filesystem::path mainExecutable;
	std::filesystem::path testsExecutable;

	std::vector<std::filesystem::path> examplesExecutables;
};

std::expected<Artifacts, CommandError> GetArtifactsByBuildType(BuildType type);
