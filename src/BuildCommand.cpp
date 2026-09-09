#include "BuildCommand.h"
#include "Utils.h"
#include <expected>
#include <filesystem>
#include <format>
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

	std::ifstream tomlFile(projectRoot / "config.toml");
	if (! tomlFile.is_open())
	{
		return CommandError{false, "Could not find config.toml file"};
	}

	Marco::TomlReader reader{};
	Marco::Toml toml = reader.Parse(tomlFile);

	tomlFile.close();

	auto cMakeFileContents = ConstructCMakeLists(toml);
	if (! cMakeFileContents.has_value())
	{
		return cMakeFileContents.error();
	}

	std::filesystem::path camkoFolderPath = projectRoot / ".camko";

	std::ofstream cMakeListsFile(camkoFolderPath / "CMakeLists.txt");

	cMakeListsFile << *cMakeFileContents;

	cMakeListsFile.close();

	return CommandError{true, "No errors occured"};
}

std::string BuildCommand::Name() const
{
	return "build";
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeLists(const Marco::Toml& toml)
{
	std::string fileContents{};

	auto projectName = GetProjectName(toml);
	if (! projectName)
	{
		return std::unexpected(projectName.error());
	}

	auto partOfCmake = ConstructCMakeProjectDefinition(toml, *projectName);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;

	partOfCmake = ConstructCMakeLanguageStandard(toml);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;

	partOfCmake = ConstructUserConfigurableOptions(toml);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;

	partOfCmake = ConstructTestOptions(toml);
	if (! partOfCmake.has_value())
	{
		return std::unexpected(partOfCmake.error());
	}

	fileContents += *partOfCmake;

	partOfCmake = ConstructBuildType();
	fileContents += *partOfCmake;

	partOfCmake = ConstructTooling();
	fileContents += *partOfCmake;

	partOfCmake = ConstructPositionIndepentendCode();
	fileContents += *partOfCmake;

	return fileContents;
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeProjectDefinition(const Marco::Toml& toml, const std::string& projectName)
{
	std::string partOfCmake{};

	auto projectSettings = toml["project"];
	if (! projectSettings || ! (*projectSettings).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}

	auto version = (*projectSettings).get()["version"];
	if (! version || ! version.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The version variable in config.toml was not set"});
	}

	partOfCmake += "cmake_minimum_required(VERSION 3.20)\n\n";

	partOfCmake += std::format("project({}\n", projectName);
	partOfCmake += std::format("\tVERSION {}\n", version.value().get().AsString()->get());

	auto description = (*projectSettings).get()["description"];
	if (description)
	{
		if (! description.value().get().IsString())
		{
			return std::unexpected(CommandError{false, "The description variable in config.toml must be a string"});
		}

		partOfCmake += std::format("DESCRIPTION {}\n", description.value().get().AsString()->get());
	}

	partOfCmake += "LANGUAGES CXX\n)\n\n";

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeLanguageStandard (const Marco::Toml& toml)
{
	std::string partOfCmake{};
	auto buildOptions = toml["build"];
	if (! buildOptions || ! (*buildOptions).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the build table in config.toml"});
	}

	auto cppVersion = (*buildOptions).get()["cpp-version"];
	if (! cppVersion || ! (*cppVersion).get().IsNumber())
	{
		return std::unexpected(CommandError{false, "Could not find the cpp-version option under the build table in config.toml"});
	}

	partOfCmake += std::format("set(CMAKE_CXX_STANDARD {})\n", (*cppVersion).get().AsNumber().value());
	partOfCmake += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
	partOfCmake += "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::ConstructUserConfigurableOptions(const Marco::Toml& toml)
{
	std::string partOfCmake{};
	auto buildOptions = toml["build"];
	if (! buildOptions || ! (*buildOptions).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the build table in config.toml"});
	}

	auto buildSharedLibs = (*buildOptions).get()["build-shared-libs"];
	if (! buildSharedLibs)
	{
		partOfCmake += "option(BUILD_SHARED_LIBS      \"Build shared libraries instead of static\" OFF)\n";
	}
	else
	{
		if (! (*buildSharedLibs).get().IsBool())
		{
			return std::unexpected(CommandError{false, "build-shared-libs in build table should be of type bool"});
		}

		partOfCmake += std::format("option(BUILD_SHARED_LIBS      \"Build shared libraries instead of static\" {})\n", (*buildSharedLibs).get().AsBool().value() ? "ON" : "OFF");
	}

	auto enableWarnings = (*buildOptions).get()["enable-warnings"];
	if (! enableWarnings)
	{
		partOfCmake += "option(ENABLE_WARNINGS        \"Enable extra compiler warnings\"           ON)\n";
	}
	else
	{
		if (! (*enableWarnings).get().IsBool())
		{
			return std::unexpected(CommandError{false, "enable-warnings in build table should be of type bool"});
		}

		partOfCmake += std::format("option(ENABLE_WARNINGS        \"Enable extra compiler warnings\"           {})\n", (*enableWarnings).get().AsBool().value() ? "ON" : "OFF");
	}

	auto warningsAsErrors = (*buildOptions).get()["warnings-as-errors"];
	if (! warningsAsErrors)
	{
		partOfCmake += "option(ENABLE_WARNINGS_AS_ERRORS \"Treat warnings as errors\"              OFF)\n";
	}
	else
	{
		if (! (*warningsAsErrors).get().IsBool())
		{
			return std::unexpected(CommandError{false, "warnings-as-errors in build table should be of type bool"});
		}

		partOfCmake += std::format("option(ENABLE_WARNINGS_AS_ERRORS \"Treat warnings as errors\"              {})\n", (*warningsAsErrors).get().AsBool().value() ? "ON" : "OFF");
	}

	auto enableSanitizers = (*buildOptions).get()["enable-sanitizers"];
	if (! enableSanitizers)
	{
		partOfCmake += "option(ENABLE_SANITIZERS      \"Build with ASan/UBSan enabled\"            OFF)\n";
	}
	else
	{
		if (! (*enableSanitizers).get().IsBool())
		{
			return std::unexpected(CommandError{false, "enable-sanitizers in build table should be of type bool"});
		}

		partOfCmake += std::format("option(ENABLE_SANITIZERS      \"Build with ASan/UBSan enabled\"            {})\n", (*enableSanitizers).get().AsBool().value() ? "ON" : "OFF");
	}

	auto enableLto = (*buildOptions).get()["enable-lto"];
	if (! enableLto)
	{
		partOfCmake += "option(ENABLE_LTO             \"Enable link-time optimization\"            OFF)\n";
	}
	else
	{
		if (! (*enableLto).get().IsBool())
		{
			return std::unexpected(CommandError{false, "benable-lto in build table should be of type bool"});
		}

		partOfCmake += std::format("option(ENABLE_LTO             \"Enable link-time optimization\"            {})\n", (*enableLto).get().AsBool().value() ? "ON" : "OFF");
	}

	auto enableCCache = (*buildOptions).get()["enable-ccache"];
	if (! enableCCache)
	{
		partOfCmake += "option(ENABLE_CCACHE          \"Use ccache if available\"                  ON)\n";
	}
	else
	{
		if (! (*enableCCache).get().IsBool())
		{
			return std::unexpected(CommandError{false, "enable-ccache in build table should be of type bool"});
		}

		partOfCmake += std::format("option(ENABLE_CCACHE          \"Use ccache if available\"                  {})\n\n", (*enableCCache).get().AsBool().value() ? "ON" : "OFF");
	}

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::ConstructTestOptions(const Marco::Toml& toml)
{
	std::string partOfCmake{};
	auto testsOptions = toml["tests"];
	if (! testsOptions || ! (*testsOptions).get().IsObject())
	{
		return std::unexpected(CommandError{false, "Could not find the tests table in config.toml"});
	}

	auto enableTests = (*testsOptions).get()["enable-tests"];
	if (! enableTests)
	{
		partOfCmake += "option(BUILD_TESTING          \"Build unit tests\"                         OFF)\n";
	}
	else
	{
		if (! (*enableTests).get().IsBool())
		{
			return std::unexpected(CommandError{false, "enable-ccache in build table should be of type bool"});
		}

		partOfCmake += std::format("option(BUILD_TESTING          \"Build unit tests\"                         {})\n\n", (*enableTests).get().AsBool().value() ? "ON" : "OFF");
	}

	return partOfCmake;
}

std::string BuildCommand::ConstructBuildType()
{
	std::string partOfCmake{};

	partOfCmake = R"(
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
)";

	partOfCmake.append("\n\n");

	return partOfCmake;
}

std::string BuildCommand::ConstructTooling()
{
	std::string partOfCmake{};

	partOfCmake = R"(
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 
if(ENABLE_CCACHE)
	find_program(CCACHE_PROGRAM ccache)
	if(CCACHE_PROGRAM)
		set(CMAKE_CXX_COMPILER_LAUNCH ${CCACHE_PROGRAM})
	endif()
endif()
)";

	partOfCmake.append("\n\n");

	return partOfCmake;
}

std::string BuildCommand::ConstructPositionIndepentendCode()
{
	std::string partOfCmake{};

	partOfCmake = R"(
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(ENABLE_LTO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
	if(ipo_supported)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	else()
		message(WARNING "LTO requested but not supported: ${ipo_error}")
	endif()
endif()
)";

	partOfCmake.append("\n\n");

	return partOfCmake;
}

std::string ConstructSantitizers()
{
	
}

std::expected<std::string, CommandError> BuildCommand::GetProjectName(const Marco::Toml& toml)
{
	auto projectSettings = toml["project"];
	if (! projectSettings)
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}

	auto projectName = (*projectSettings).get()["name"];
	if (! projectName || ! projectName.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in config.toml not set or is not a string"});
	}

	return (*projectName).get().AsString()->get();
}
