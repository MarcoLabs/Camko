#pragma once

#include "../CommandError.h"
#include <expected>
#include <filesystem>
#include <string_view>
#include <system_error>

namespace utils
{
	std::expected<std::filesystem::path, CommandError> GetCamkoProjectRootDirectory();
	CommandError FillConfigFile(const std::string_view& content); // requires a string with a null terminator

	std::filesystem::path GetBuildFolderPath(const std::filesystem::path& projectRoot, const std::string& buildSystem);
	std::error_code RemoveAllFoldersFrom(const std::filesystem::path& path);
}