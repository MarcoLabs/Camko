#include "Utils.h"
#include "CommandError.h"
#include "marco/utils/FileUtils.h"
#include <expected>
#include <filesystem>
#include <system_error>

std::expected<std::filesystem::path, CommandError> utils::GetCamkoProjectRootDirectory()
{
	std::filesystem::path currentPath = std::filesystem::current_path();

	std::error_code error{};
	
	while (! currentPath.empty())
	{
		std::filesystem::path camkoDirPath = currentPath / ".camko";

		if (std::filesystem::is_directory(camkoDirPath, error))
		{
			return currentPath;
		}

		if (error && error != std::errc::no_such_file_or_directory)
		{
			return std::unexpected(CommandError{false, "OS error accessing path '" + currentPath.string() + "': " + error.message()});
		}

		std::filesystem::path parent = currentPath.parent_path();
		if (parent == currentPath)
		{
			break;
		}

		currentPath = std::move(parent);
	}

	return std::unexpected(CommandError{false, "Error: Could not find an active camko project"});
}

CommandError utils::FillConfigFile(const std::string& content)
{
	auto projectRoot = GetCamkoProjectRootDirectory();
	if (! projectRoot)
	{
		return projectRoot.error();
	}

	Marco::WriteFile(*projectRoot / "config.toml", content);

	return CommandError{true, "No errors occured"};
}
