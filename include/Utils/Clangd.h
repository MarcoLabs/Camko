#pragma once

#include "CommandError.h"
#include <filesystem>
#include <string>


namespace utils
{
	CommandError CreateClangdFile(const std::filesystem::path& projectPath, const std::string& buildSystem);
};
