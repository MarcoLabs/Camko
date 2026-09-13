#include "BuildCommand.h"
#include "CommandError.h"
#include "marco/utils/FileUtils.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <system_error>

// ---------------------------------------------------------------------------
// NOTE ON TEST STRATEGY
// ---------------------------------------------------------------------------
// BuildCommand's private static helpers are tested indirectly through Execute()
// using different config.toml contents, checking the returned CommandError and
// generated .camko/CMakeLists.txt. Omitting [build].type causes BuildProject to
// fail predictably after CMakeLists generation but before std::system(...),
// keeping the tests hermetic; a disabled integration test covers the full path.
// ---------------------------------------------------------------------------

namespace
{
	bool Contains(const std::string& haystack, const std::string& needle)
	{
		return haystack.find(needle) != std::string::npos;
	}
}

class BuildCommandTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		this->m_originalCwd = std::filesystem::current_path();

		std::string testHash = std::to_string(std::hash<std::string>{}(
				::testing::UnitTest::GetInstance()->current_test_info()->name())
			) + "_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed());

		this->m_tempDir = std::filesystem::temp_directory_path() / std::filesystem::path("camko_test_" + testHash);

		std::filesystem::remove_all(this->m_tempDir);
		std::filesystem::create_directories(this->m_tempDir / ".camko");
		std::filesystem::current_path(this->m_tempDir);
	}

	void TearDown() override
	{
		std::filesystem::current_path(this->m_originalCwd);

		std::error_code ec{};
		std::filesystem::remove_all(this->m_tempDir, ec);
	}

	void WriteConfig(const std::string& content)
	{
		std::ofstream configFile(this->m_tempDir / "config.toml");
		configFile << content;
	}

	std::string ReadGeneratedCMakeLists()
	{
		return Marco::ReadFile(this->m_tempDir / ".camko" / "CMakeLists.txt");
	}

	static constexpr const char* kMinimalValidConfig = R"(
[project]
name = "myapp"
version = "1.0.0"

[build]
cpp-version = 23
source-directory = "src"
header-directory = "include"

[tests]
enable-tests = false
)";

	std::filesystem::path m_originalCwd{};
	std::filesystem::path m_tempDir{};
	BuildCommand m_command{};
};

TEST_F(BuildCommandTest, NameReturnsBuild)
{
	EXPECT_EQ(this->m_command.Name(), "build");
}

TEST_F(BuildCommandTest, NoCamkoDirectory_ReturnsError)
{
	std::filesystem::remove_all(this->m_tempDir / ".camko");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Error: Could not find an active camko project");
}

TEST_F(BuildCommandTest, MissingConfigToml_ReturnsError)
{
	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the config.toml file");
}

TEST_F(BuildCommandTest, MissingProjectTable_ReturnsError)
{
	WriteConfig(R"(
[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the project table in config.toml");
}

TEST_F(BuildCommandTest, MissingProjectName_ReturnsError)
{
	WriteConfig(R"(
[project]
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The name variable in config.toml was not set or is not a string");
}

TEST_F(BuildCommandTest, MissingProjectVersion_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"

[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);

	EXPECT_EQ(result.message, "The version variable in config.toml was not set or is not a string");
}

TEST_F(BuildCommandTest, NonStringVersionReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = 1

[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The version variable in config.toml was not set or is not a string");
}

TEST_F(BuildCommandTest, NonStringDescription_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"
description = 1

[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The description variable in config.toml must be a string");
}

TEST_F(BuildCommandTest, ValidProjectDefinition_WritesExpectedCMakeContent)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"
description = "Test app"

[build]
cpp-version = 23

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "cmake_minimum_required(VERSION 3.20)"));
	EXPECT_TRUE(Contains(cmakeLists, "project(\"my_app\""));
	EXPECT_TRUE(Contains(cmakeLists, "VERSION \"1.0.0\""));
	EXPECT_TRUE(Contains(cmakeLists, "DESCRIPTION \"Test app\""));
}

TEST_F(BuildCommandTest, ProjectDefinitionWithoutDescription_OmitsDescriptionLine)
{
	WriteConfig(kMinimalValidConfig);

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_FALSE(Contains(cmakeLists, "DESCRIPTION"));
}

TEST_F(BuildCommandTest, MissingBuildTable_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the build table in config.toml");
}

TEST_F(BuildCommandTest, MissingCppVersion_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
type = "Release"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the cpp-version option in the build table in config.toml");
}

TEST_F(BuildCommandTest, NonNumericCppVersion_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = "23"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the cpp-version option in the build table in config.toml");
}

TEST_F(BuildCommandTest, ValidCppVersion_WritesLanguageStandardLines)
{
	WriteConfig(kMinimalValidConfig);

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "set(CMAKE_CXX_STANDARD 23)"));
	EXPECT_TRUE(Contains(cmakeLists, "set(CMAKE_CXX_STANDARD_REQUIRED ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "set(CMAKE_CXX_EXTENSIONS OFF)"));
}

TEST_F(BuildCommandTest, OmittedBooleanOptions_UseDocumentedDefaults)
{
	WriteConfig(kMinimalValidConfig);

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "option(BUILD_SHARED_LIBS      \"Build shared libraries instead of static\" OFF)"));
	EXPECT_TRUE(Contains(cmakeLists, "option(ENABLE_WARNINGS        \"Enable extra compiler warnings\"           ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "option(ENABLE_WARNINGS_AS_ERRORS \"Treat warnings as errors\"              OFF)"));
	EXPECT_TRUE(Contains(cmakeLists, "option(ENABLE_SANITIZERS      \"Build with ASan/UBSan enabled\"            OFF)"));
	EXPECT_TRUE(Contains(cmakeLists, "option(ENABLE_LTO             \"Enable link-time optimization\"            OFF)"));
	EXPECT_TRUE(Contains(cmakeLists, "option(ENABLE_CCACHE          \"Use ccache if available\"                  ON)"));
}

TEST_F(BuildCommandTest, ExplicitBooleanOptions_AreHonored)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
build-shared-libs = true
enable-warnings = false
warnings-as-errors = true
enable-sanitizers = true
enable-lto = true
enable-ccache = false

[tests]
enable-tests = false
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "\"Build shared libraries instead of static\" ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "\"Enable extra compiler warnings\"           OFF)"));
	EXPECT_TRUE(Contains(cmakeLists, "\"Treat warnings as errors\"              ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "\"Build with ASan/UBSan enabled\"            ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "\"Enable link-time optimization\"            ON)"));
	EXPECT_TRUE(Contains(cmakeLists, "\"Use ccache if available\"                  OFF)"));
}

TEST_F(BuildCommandTest, NonBooleanBuildSharedLibs_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
build-shared-libs = "true"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The build-shared-libs option in the build table must be a boolean");
}

TEST_F(BuildCommandTest, NonBooleanEnableSanitizers_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
enable-sanitizers = "true"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The enable-sanitizers option in the build table must be a boolean");
}

TEST_F(BuildCommandTest, MissingTestsTable_DoesNotErrorAndOmitsTestsOption)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the type field in the build table in config.toml");

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_FALSE(Contains(cmakeLists, "option(CAMKO_ENABLE_TESTS"));
}

TEST_F(BuildCommandTest, TestsKeyNotATable_ReturnsError)
{
	WriteConfig(R"(
tests = "not-a-table"

[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the tests table in config.toml");
}

TEST_F(BuildCommandTest, NonBooleanEnableTests_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = "false"
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The enable-tests option in the tests table must be a boolean");
}

TEST_F(BuildCommandTest, OmittedEnableTests_DefaultsToOff)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
tests-directory = "tests"
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "option(CAMKO_ENABLE_TESTS          \"Build unit tests\"                         OFF)"));
}

TEST_F(BuildCommandTest, EnableTestsTrue_WritesOnOption)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = true
tests-directory = "tests"
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "option(CAMKO_ENABLE_TESTS          \"Build unit tests\"                         ON)"));
}

TEST_F(BuildCommandTest, NoDependenciesArray_ProducesNoDependencyBlocks)
{
	WriteConfig(kMinimalValidConfig);

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_FALSE(Contains(cmakeLists, "find_package"));

	EXPECT_FALSE(Contains(cmakeLists, "target_link_libraries(camko_core PUBLIC "));
	EXPECT_FALSE(Contains(cmakeLists, "target_link_libraries(camko_core INTERFACE fmt"));
}

TEST_F(BuildCommandTest, FindPackageDependency_WritesFindPackageBlock)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
find-package-name = "OpenSSL"
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "find_package(OpenSSL REQUIRED)"));
	EXPECT_TRUE(Contains(cmakeLists, "target_link_libraries(camko_core PUBLIC OpenSSL::OpenSSL)"));
	EXPECT_TRUE(Contains(cmakeLists, "target_link_libraries(camko_core INTERFACE OpenSSL::OpenSSL)"));
}

TEST_F(BuildCommandTest, FetchContentDependency_DefaultsLinkTargetToNameName)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
name = "fmt"
repo = "https://github.com/fmtlib/fmt.git"
version = "10.1.1"
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "FetchContent_Declare("));
	EXPECT_TRUE(Contains(cmakeLists, "fmt"));
	EXPECT_TRUE(Contains(cmakeLists, "GIT_REPOSITORY https://github.com/fmtlib/fmt.git"));
	EXPECT_TRUE(Contains(cmakeLists, "GIT_TAG 10.1.1"));
	EXPECT_TRUE(Contains(cmakeLists, "target_link_libraries(camko_core PUBLIC fmt::fmt)"));
}

TEST_F(BuildCommandTest, FetchContentDependency_ExplicitLinkTargetIsUsed)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
name = "marco"
repo = "https://github.com/MarcoLabs/MarcoLib"
link-target = "Marco::Toml"
)");

	this->m_command.Execute({});

	std::string cmakeLists = ReadGeneratedCMakeLists();
	EXPECT_TRUE(Contains(cmakeLists, "target_link_libraries(camko_core PUBLIC Marco::Toml)"));
	EXPECT_TRUE(Contains(cmakeLists, "target_link_libraries(camko_core INTERFACE Marco::Toml)"));

	EXPECT_TRUE(Contains(cmakeLists, "GIT_REPOSITORY https://github.com/MarcoLabs/MarcoLib\n)"));
}

TEST_F(BuildCommandTest, DependencyMissingNameAndFindPackageName_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
repo = "https://github.com/fmtlib/fmt.git"
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The name in the dependencies array does not exist or isnt a string");
}

TEST_F(BuildCommandTest, DependencyMissingRepo_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
name = "fmt"
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The repo in the dependencies array does not exist or isnt a string");
}

TEST_F(BuildCommandTest, DependencyNonStringVersion_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
name = "fmt"
repo = "https://github.com/fmtlib/fmt.git"
version = 10
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The version in the dependencies array isnt a string");
}

TEST_F(BuildCommandTest, DependencyNonStringLinkTarget_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23

[tests]
enable-tests = false

[[dependencies]]
name = "fmt"
repo = "https://github.com/fmtlib/fmt.git"
link-target = 5
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "The link-target in the dependencies array isnt a string");
}

TEST_F(BuildCommandTest, MissingBuildTypeField_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
source-directory = "src"
header-directory = "include"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the type field in the build table in config.toml");
}

TEST_F(BuildCommandTest, MissingSourceDirectoryField_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
type = "Release"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the source-directory field in the build table in config.toml");
}

TEST_F(BuildCommandTest, MissingHeaderDirectoryField_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
type = "Release"
source-directory = "src"

[tests]
enable-tests = false
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the header-directory field in the build table in config.toml");
}

TEST_F(BuildCommandTest, TestsEnabledWithoutTestsDirectory_ReturnsError)
{
	WriteConfig(R"(
[project]
name = "my_app"
version = "1.0.0"

[build]
cpp-version = 23
type = "Release"
source-directory = "src"
header-directory = "include"

[tests]
enable-tests = true
)");

	CommandError result = this->m_command.Execute({});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Could not find the tests-directory field in the tests table in config.toml");
}
