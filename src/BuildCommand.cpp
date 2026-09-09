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
		return CommandError{false, "Could not find the config.toml file"};
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

	CommandError buildResult = BuildProject(toml);
	if (! buildResult.valid)
	{
		return buildResult;
	}

	return CommandError{true, "No errors occurred"};
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

	partOfCmake = ConstructPositionIndependentCode();
	fileContents += *partOfCmake;

	partOfCmake = ConstructCompilerWarnings();
	fileContents += *partOfCmake;
	
	partOfCmake = ConstructSantitizers();
	fileContents += *partOfCmake;

	partOfCmake = ConstructSourceFiles();
	fileContents += *partOfCmake;

	partOfCmake = ConstructTesting();
	fileContents += *partOfCmake;

	partOfCmake = ConstructInstallRules();
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
		return std::unexpected(CommandError{false, "The version variable in config.toml was not set or is not a string"});
	}

	partOfCmake += "cmake_minimum_required(VERSION 3.20)\n\n";

	partOfCmake += std::format("project(\"{}\"\n", projectName);
	partOfCmake += std::format("\tVERSION \"{}\"\n", version.value().get().AsString()->get());

	auto description = (*projectSettings).get()["description"];
	if (description)
	{
		if (! description.value().get().IsString())
		{
			return std::unexpected(CommandError{false, "The description variable in config.toml must be a string"});
		}

		partOfCmake += std::format("\tDESCRIPTION \"{}\"\n", description.value().get().AsString()->get());
	}

	partOfCmake += "\tLANGUAGES CXX\n)\n\n";

	return partOfCmake;
}

std::expected<std::string, CommandError> BuildCommand::ConstructCMakeLanguageStandard(const Marco::Toml& toml)
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
		return std::unexpected(CommandError{false, "Could not find the cpp-version option in the build table in config.toml"});
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
			return std::unexpected(CommandError{false, "The build-shared-libs option in the build table must be a boolean"});
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
			return std::unexpected(CommandError{false, "The enable-warnings option in the build table must be a boolean"});
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
			return std::unexpected(CommandError{false, "The warnings-as-errors option in the build table must be a boolean"});
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
			return std::unexpected(CommandError{false, "The enable-sanitizers option in the build table must be a boolean"});
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
			return std::unexpected(CommandError{false, "The enable-lto option in the build table must be a boolean"});
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
			return std::unexpected(CommandError{false, "The enable-ccache option in the build table must be a boolean"});
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
		partOfCmake += "option(CAMKO_ENABLE_TESTS          \"Build unit tests\"                         OFF)\n";
	}
	else
	{
		if (! (*enableTests).get().IsBool())
		{
			return std::unexpected(CommandError{false, "The enable-tests option in the tests table must be a boolean"});
		}

		partOfCmake += std::format("option(CAMKO_ENABLE_TESTS          \"Build unit tests\"                         {})\n\n", (*enableTests).get().AsBool().value() ? "ON" : "OFF");
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

	partOfCmake.push_back('\n');

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
		set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	endif()
endif()
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}

std::string BuildCommand::ConstructPositionIndependentCode()
{
	std::string partOfCmake{};

	partOfCmake = R"(
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}

std::string BuildCommand::ConstructCompilerWarnings()
{
	std::string partOfCmake{};

	partOfCmake = R"(
add_library(project_warnings INTERFACE)

if(ENABLE_WARNINGS)
	if(MSVC)
		target_compile_options(project_warnings INTERFACE /W4)
		if(ENABLE_WARNINGS_AS_ERRORS)
			target_compile_options(project_warnings INTERFACE /WX)
		endif()
	else()
		target_compile_options(project_warnings INTERFACE
			-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
		)
		if(ENABLE_WARNINGS_AS_ERRORS)
			target_compile_options(project_warnings INTERFACE -Werror)
		endif()
	endif()
endif()
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}

std::string BuildCommand::ConstructSantitizers()
{
	std::string partOfCmake{};

	partOfCmake = R"(
add_library(project_sanitizers INTERFACE)

if(ENABLE_SANITIZERS AND NOT MSVC)
	target_compile_options(project_sanitizers INTERFACE
		-fsanitize=address,undefined -fno-omit-frame-pointer
	)
	target_link_options(project_sanitizers INTERFACE
		-fsanitize=address,undefined
	)
endif()
)";

	partOfCmake.append("\n\n");

	return partOfCmake;
}

std::string BuildCommand::ConstructSourceFiles()
{
	std::string partOfCmake{};

	partOfCmake = R"(
set(CAMKO_SOURCE_DIR "src" CACHE STRING "Directory containing .cpp source files")
set(CAMKO_HEADER_DIR "include" CACHE STRING "Directory containing .h header files")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_SOURCE_DIR}")
	message(FATAL_ERROR
		"The source directory '${CAMKO_SOURCE_DIR}' does not exist."
	)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_SOURCE_DIR}/main.cpp")
	message(FATAL_ERROR
		"The main.cpp file does not exist in the source directory "
		"'${CAMKO_SOURCE_DIR}'."
	)
endif()

file(GLOB_RECURSE CAMKO_ALL_SOURCES CONFIGURE_DEPENDS
	"${CAMKO_SOURCE_DIR}/*.cpp"
)
list(FILTER CAMKO_ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

if(CAMKO_ALL_SOURCES)
	add_library(camko_core ${CAMKO_ALL_SOURCES})
else()
	add_library(camko_core INTERFACE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_HEADER_DIR}")
	if(CAMKO_ALL_SOURCES)
		target_include_directories(camko_core
			PUBLIC
				${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_HEADER_DIR}
		)
	else()
		target_include_directories(camko_core
			INTERFACE
				${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_HEADER_DIR}
		)
	endif()
endif()

target_link_libraries(camko_core
	INTERFACE
		project_warnings
		project_sanitizers
)

add_executable(${PROJECT_NAME} ${CAMKO_SOURCE_DIR}/main.cpp)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		camko_core
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD ${CMAKE_CXX_STANDARD}
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

if(TARGET camko_core)
	set_target_properties(camko_core PROPERTIES
		CXX_STANDARD ${CMAKE_CXX_STANDARD}
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)
endif()
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}

std::string BuildCommand::ConstructTesting()
{
	std::string partOfCmake{};

	partOfCmake = R"(
set(CAMKO_TESTS_DIR "tests" CACHE STRING "Directory containing *_test.cpp files")

if(CAMKO_ENABLE_TESTS)
	if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_TESTS_DIR}")
		message(FATAL_ERROR
			"Tests are enabled but the tests directory "
			"'${CAMKO_TESTS_DIR}' does not exist."
		)
	endif()

	enable_testing()

	file(GLOB_RECURSE CAMKO_TEST_SOURCES CONFIGURE_DEPENDS
		"${CAMKO_TESTS_DIR}/*_test.cpp"
	)

	if(CAMKO_TEST_SOURCES)
		include(FetchContent)
		FetchContent_Declare(
			googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG v1.14.0
		)
		if(MSVC)
			set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
		endif()
		FetchContent_MakeAvailable(googletest)

		add_executable(camko_tests ${CAMKO_TEST_SOURCES})

		target_link_libraries(camko_tests
			PRIVATE
				camko_core
				GTest::gtest_main
				project_warnings
				project_sanitizers
		)

		set_target_properties(camko_tests PROPERTIES
			CXX_STANDARD ${CMAKE_CXX_STANDARD}
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
		)

		include(GoogleTest)
		gtest_discover_tests(camko_tests)
	else()
		message(STATUS
			"Tests are enabled but no '*_test.cpp' files were found in "
			"'${CAMKO_TESTS_DIR}' - skipping test target."
		)
	endif()
endif()
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}

std::string BuildCommand::ConstructInstallRules()
{
	std::string partOfCmake{};

	partOfCmake = R"(
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} camko_core
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
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
		return std::unexpected(CommandError{false, "The name variable in config.toml was not set or is not a string"});
	}

	return (*projectName).get().AsString()->get();
}

CommandError BuildCommand::BuildProject(const Marco::Toml& toml)
{
	auto buildOptions = toml["build"];
	if (!buildOptions)
	{
		return CommandError{false, "Could not find the build table in config.toml"};
	}

	auto testOptions = toml["tests"];
	if (!testOptions)
	{
		return CommandError{false, "Could not find the tests table in config.toml"};
	}

	auto buildType = (*buildOptions).get()["type"];
	if (!buildType || !(*buildType).get().IsString())
	{
		return CommandError{false, "Could not find the type field in the build table in config.toml"};
	}

	auto sourceDir = (*buildOptions).get()["source-directory"];
	if (!sourceDir || !(*sourceDir).get().IsString())
	{
		return CommandError{false, "Could not find the source-directory field in the build table in config.toml"};
	}

	auto headerDir = (*buildOptions).get()["header-directory"];
	if (!headerDir || !(*headerDir).get().IsString())
	{
		return CommandError{false, "Could not find the header-directory field in the build table in config.toml"};
	}

	auto enableTests = (*testOptions).get()["enable-tests"];
	if (!enableTests || !(*enableTests).get().IsBool())
	{
		return CommandError{false, "Could not find the enable-tests field in the tests table in config.toml"};
	}

	bool testsAreEnabled = enableTests.value().get().AsBool().value();

	std::string testsDirValue = "tests";
	if (testsAreEnabled)
	{
		auto testsDir = (*testOptions).get()["tests-directory"];
		if (!testsDir || !(*testsDir).get().IsString())
		{
			return CommandError{false, "Could not find the tests-directory field in the tests table in config.toml"};
		}
		
		testsDirValue = testsDir.value().get().AsString().value();
	}

	std::string configureCmd = std::format("cmake -S .camko -B .camko/build -DCMAKE_BUILD_TYPE={} -DCAMKO_SOURCE_DIR=../{} -DCAMKO_HEADER_DIR=../{} -DCAMKO_ENABLE_TESTS={}",
		buildType.value().get().AsString().value().get(),
		sourceDir.value().get().AsString().value().get(),
		headerDir.value().get().AsString().value().get(),
		testsAreEnabled ? "ON" : "OFF"
	);
	
	if (testsAreEnabled)
	{
		configureCmd += std::format(" -DCAMKO_TESTS_DIR=../{}", testsDirValue);
	}

	std::system(configureCmd.c_str());
	std::system("cmake --build .camko/build");

	return CommandError{true, ""};
}