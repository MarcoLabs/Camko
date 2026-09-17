#pragma once

#include "CommandError.h"
#include <expected>
#include <filesystem>

namespace utils
{
	std::expected<std::filesystem::path, CommandError> GetCamkoProjectRootDirectory();
	CommandError FillConfigFile(const std::string_view& content); // requires a string with a null terminator
}