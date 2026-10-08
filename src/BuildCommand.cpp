#include "BuildCommand.h"
#include "CommandError.h"
#include "Config.h"
#include "Utils/Clangd.h"
#include "Utils/General.h"
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

CommandError BuildCommand::Execute(const Config& config, const std::vector<std::string>& args)
{
	auto result = utils::GetCamkoProjectRootDirectory();
	if (! result.has_value())
	{
		return result.error();
	}

	std::filesystem::path projectRoot = result.value();

	ConstructCMakeLists(projectRoot, config);

	CommandError buildResult = BuildProject(config, projectRoot);
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

CommandError BuildCommand::BuildProject(const Config& config, const std::filesystem::path& projectRoot)
{
	utils::CreateClangdFile(projectRoot, config.buildConfig.buildSystem);

	const auto camkoDir = projectRoot / ".camko";
	const auto buildDir = utils::GetBuildFolderPath(projectRoot, config.buildConfig.buildSystem);

	CommandError result = ConfigureProject (config, camkoDir, buildDir);
	if (! result.valid)
	{
		return result;
	}

	BuildCmakeProject(buildDir);

	return CommandError{true, "No errors occured"};
}

CommandError BuildCommand::ConfigureProject(const Config& config, const std::filesystem::path& camkoDir, const std::filesystem::path& buildDir)
{
	if (! std::filesystem::exists(buildDir))
	{
		std::error_code ec = utils::RemoveAllFoldersFrom(buildDir.parent_path()); // buildDir.parent_path() returns the .camko/build folder
		if (ec)
		{
			return CommandError{false, std::format("Error while trying to delete old build caches. Error message: {}", ec.message())};
		}

		if (! std::filesystem::create_directories(buildDir, ec) && ec)
		{
			return CommandError{false, std::format("Could not create a subfolder at: {}\nError message: {}", std::filesystem::absolute(buildDir).string(), ec.message())};
		}
	}

	std::string configureCmd = std::format(
		"cmake -S \"{}\" -B \"{}\" "
		"-DCMAKE_BUILD_TYPE={} "
		"-DCAMKO_SOURCE_DIR=\"../{}\" "
		"-DCAMKO_HEADER_DIR=\"../{}\" "
		"-DCAMKO_ENABLE_TESTS={} "
		"-DCAMKO_ENABLE_EXAMPLES={}",
		camkoDir.string(),
		buildDir.string(),
		Config::BuildTypeToString(config.buildConfig.type),
		config.buildConfig.sourceDirectory,
		config.buildConfig.headerDirectory,
		(config.testsConfig    && config.testsConfig->enableTests)       ? "ON" : "OFF",
		(config.examplesConfig && config.examplesConfig->enableExamples) ? "ON" : "OFF"
	);

	if (config.testsConfig && config.testsConfig->enableTests)
	{
		configureCmd += std::format(" -DCAMKO_TESTS_DIR=\"../{}\"", config.testsConfig->testsDirectory);
	}

	if (config.examplesConfig && config.examplesConfig->enableExamples)
	{
		configureCmd += std::format(" -DCAMKO_EXAMPLES_PATH=\"../{}\"", config.examplesConfig->examplesDirectory);
	}

	configureCmd += std::format(" -G \"{}\"", config.buildConfig.buildSystem);

	std::system(configureCmd.c_str());

	return CommandError{true, "No errors occured"};
}

void BuildCommand::BuildCmakeProject(const std::filesystem::path& buildDir)
{
	std::string buildCmd = std::format(
		"cmake --build \"{}\"",
		buildDir.string()
	);

	std::system(buildCmd.c_str());
}

void BuildCommand::ConstructCMakeLists(const std::filesystem::path& projectRoot, const Config& config)
{
	std::string fileContents{};

	std::string partOfCmake = ConstructCMakeProjectDefinition(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructCMakeLanguageStandard(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructUserConfigurableOptions(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructTestOptions(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructExamplesOptions(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructBuildType();
	fileContents += partOfCmake;

	partOfCmake = ConstructTooling();
	fileContents += partOfCmake;

	partOfCmake = ConstructPositionIndependentCode();
	fileContents += partOfCmake;

	partOfCmake = ConstructCompilerWarnings();
	fileContents += partOfCmake;

	partOfCmake = ConstructSantitizers();
	fileContents += partOfCmake;

	partOfCmake = ConstructSourceFiles();
	fileContents += partOfCmake;

	partOfCmake = ConstructDependencies(config);
	fileContents += partOfCmake;

	partOfCmake = ConstructTesting();
	fileContents += partOfCmake;

	partOfCmake = ConstructExamples();
	fileContents += partOfCmake;

	partOfCmake = ConstructInstallRules();
	fileContents += partOfCmake;

	partOfCmake = ConstructArtifactsFiles();
	fileContents += partOfCmake;

	std::filesystem::path camkoFolderPath = projectRoot / ".camko";

	std::ofstream cMakeListsFile(camkoFolderPath / "CMakeLists.txt");

	cMakeListsFile << fileContents;
	cMakeListsFile.close();
}

std::string BuildCommand::ConstructCMakeProjectDefinition(const Config& config)
{
	std::string partOfCmake{};

	partOfCmake += "cmake_minimum_required(VERSION 3.20)\n\n";

	partOfCmake += std::format("project(\"{}\"\n", config.projectConfig.name);
	partOfCmake += std::format("\tVERSION \"{}\"\n", config.projectConfig.version);

	partOfCmake += std::format("\tDESCRIPTION \"{}\"\n", config.projectConfig.description);
	partOfCmake += "\tLANGUAGES CXX\n)\n\n";

	return partOfCmake;
}

std::string BuildCommand::ConstructCMakeLanguageStandard(const Config& config)
{
	std::string partOfCmake{};

	partOfCmake += std::format("set(CMAKE_CXX_STANDARD {})\n", config.buildConfig.cppVersion);
	partOfCmake += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
	partOfCmake += "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

	return partOfCmake;
}

std::string BuildCommand::ConstructUserConfigurableOptions(const Config& config)
{
	std::string partOfCmake{};

	partOfCmake += std::format("option(BUILD_SHARED_LIBS      \"Build shared libraries instead of static\" {})\n", config.buildConfig.buildSharedLibs ? "ON" : "OFF");
	partOfCmake += std::format("option(ENABLE_WARNINGS        \"Enable extra compiler warnings\"           {})\n", config.buildConfig.enableWarnings ? "ON" : "OFF");
	partOfCmake += std::format("option(ENABLE_WARNINGS_AS_ERRORS \"Treat warnings as errors\"              {})\n", config.buildConfig.warningsAsErrors ? "ON" : "OFF");
	partOfCmake += std::format("option(ENABLE_SANITIZERS      \"Build with ASan/UBSan enabled\"            {})\n", config.buildConfig.enableSanitizers ? "ON" : "OFF");
	partOfCmake += std::format("option(ENABLE_LTO             \"Enable link-time optimization\"            {})\n", config.buildConfig.enableLto ? "ON" : "OFF");
	partOfCmake += std::format("option(ENABLE_CCACHE          \"Use ccache if available\"                  {})\n\n", config.buildConfig.enableCcache ? "ON" : "OFF");
	
	partOfCmake += "set(CAMKO_ARTIFACTS_DIR \"${CMAKE_SOURCE_DIR}/artifacts\" CACHE PATH \"Directory for Camko artifact metadata\")\n";

	partOfCmake += "file(MAKE_DIRECTORY \"${CAMKO_ARTIFACTS_DIR}\")\n";


	return partOfCmake;
}

std::string BuildCommand::ConstructTestOptions(const Config& config)
{
	if (! config.testsConfig)
	{
		return "";
	}

	std::string partOfCmake{};

	partOfCmake += std::format("option(CAMKO_ENABLE_TESTS          \"Build unit tests\"                         {})\n\n", config.testsConfig->enableTests ? "ON" : "OFF");

	return partOfCmake;
}

std::string BuildCommand::ConstructExamplesOptions(const Config& config)
{
	std::string partOfCmake{};

	if (! config.examplesConfig)
	{
		return "";
	}

	partOfCmake += std::format("option(CAMKO_ENABLE_EXAMPLES       \"Build examples\"                           {})\n\n", config.examplesConfig->enableExamples ? "ON" : "OFF");


	return partOfCmake;
}

std::string BuildCommand::ConstructDependencies(const Config& config)
{
	std::string partOfCmake{};

	if (! config.dependenciesConfig)
	{
		return "";
	}

	for (const auto& dependency : *config.dependenciesConfig)
	{
		if (const auto* smallDependencyConfig = std::get_if<SmallDependencyConfig>(&dependency))
		{
			partOfCmake += std::format(R"(
find_package({0} REQUIRED)
if(CAMKO_ALL_SOURCES)
	target_link_libraries(camko_core PUBLIC {0}::{0})
else()
	target_link_libraries(camko_core INTERFACE {0}::{0})
endif()
)", smallDependencyConfig->findPackageName);

			partOfCmake.push_back('\n');
			continue;
		}

		const auto* regularDependencyConfig = std::get_if<RegularDependencyConfig>(&dependency);

		partOfCmake += "include(FetchContent)\n\n";
		partOfCmake += "FetchContent_Declare(\n";
		partOfCmake += '\t' + regularDependencyConfig->name + '\n';
		partOfCmake += "\tGIT_REPOSITORY " + regularDependencyConfig->repo + '\n';

		if (! regularDependencyConfig->version.empty())
		{
			partOfCmake += "\tGIT_TAG " + regularDependencyConfig->version + '\n';
		}

		partOfCmake += ")\nFetchContent_MakeAvailable(" + regularDependencyConfig->name + ")\n\n";

		partOfCmake += "if(CAMKO_ALL_SOURCES)\n";
		partOfCmake += "\ttarget_link_libraries(camko_core PUBLIC " + regularDependencyConfig->linkTarget + ")\n";
		partOfCmake += "else()\n";
		partOfCmake += "\ttarget_link_libraries(camko_core INTERFACE " + regularDependencyConfig->linkTarget + ")\n";
		partOfCmake += "endif()\n\n";
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

set(CAMKO_ARTIFACTS
"[targets.${PROJECT_NAME}]
type = \"executable\"
path = \"$<TARGET_FILE:${PROJECT_NAME}>\"

"
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

		add_executable(${PROJECT_NAME}_tests ${CAMKO_TEST_SOURCES})

		string(APPEND CAMKO_ARTIFACTS
"[targets.${PROJECT_NAME}_tests]
type = \"test\"
path = \"$<TARGET_FILE:${PROJECT_NAME}_tests>\"

		"
		)

		target_link_libraries(${PROJECT_NAME}_tests
			PRIVATE
				camko_core
				GTest::gtest_main
				project_warnings
				project_sanitizers
		)

		set_target_properties(${PROJECT_NAME}_tests PROPERTIES
			CXX_STANDARD ${CMAKE_CXX_STANDARD}
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
		)

		include(GoogleTest)
		gtest_discover_tests(${PROJECT_NAME}_tests)
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

std::string BuildCommand::ConstructExamples()
{
	std::string partOfCmake = R"(
set(CAMKO_EXAMPLES_PATH "examples" CACHE STRING "Directory containing all exammples files")

if(CAMKO_ENABLE_EXAMPLES)
	file(GLOB_RECURSE CAMKO_EXAMPLE_SOURCES CONFIGURE_DEPENDS
	"${CAMKO_EXAMPLES_PATH}/main.cpp"
	)

	foreach(CAMKO_EXAMPLE_SRC ${CAMKO_EXAMPLE_SOURCES})
		file(RELATIVE_PATH CAMKO_EXAMPLE_REL
			"${CMAKE_CURRENT_SOURCE_DIR}/${CAMKO_EXAMPLES_PATH}"
			"${CAMKO_EXAMPLE_SRC}"
		)

		get_filename_component(CAMKO_EXAMPLE_DIR
			"${CAMKO_EXAMPLE_REL}"
			DIRECTORY
		)

		string(REPLACE "/" "_" CAMKO_EXAMPLE_NAME
			"${CAMKO_EXAMPLE_DIR}"
		)

		set(CAMKO_EXAMPLE_TARGET "${CAMKO_EXAMPLE_NAME}_example")

		add_executable(${CAMKO_EXAMPLE_TARGET} "${CAMKO_EXAMPLE_SRC}")

		string(APPEND CAMKO_ARTIFACTS
"[targets.${CAMKO_EXAMPLE_TARGET}]
type = \"example\"
path = \"$<TARGET_FILE:${CAMKO_EXAMPLE_TARGET}>\"

		"
		)

		target_link_libraries(${CAMKO_EXAMPLE_TARGET}
			PRIVATE
				camko_core)

		set_target_properties(${CAMKO_EXAMPLE_TARGET} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
		)
		endforeach()
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

std::string BuildCommand::ConstructArtifactsFiles()
{
	std::string partOfCmake = R"(
file(GENERATE
	OUTPUT "${CAMKO_ARTIFACTS_DIR}/$<CONFIG>.toml"
	CONTENT "${CAMKO_ARTIFACTS}"
)
)";

	partOfCmake.push_back('\n');

	return partOfCmake;
}
