#include "CommandError.h"
#include "Defaults.h"
#include "InitCommand.h"
#include "marco/utils/FileUtils.h"
#include <filesystem>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <system_error>


class InitCommandTest : public ::testing::Test
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
		std::filesystem::create_directories(this->m_tempDir);
		std::filesystem::current_path(this->m_tempDir);
	}

	void TearDown() override
	{
		std::filesystem::current_path(this->m_originalCwd);

		std::error_code ec{};
		std::filesystem::remove_all(this->m_tempDir, ec);
	}

	std::filesystem::path m_originalCwd{};
	std::filesystem::path m_tempDir{};
	InitCommand m_command{};
};

TEST_F(InitCommandTest, NameReturnsInit)
{
	EXPECT_EQ(this->m_command.Name(), "init");
}

TEST_F(InitCommandTest, NoArgs_CreatesProjectInCurrentDirectory)
{
	CommandError result = this->m_command.Execute({});

	EXPECT_TRUE(result.valid);

	EXPECT_TRUE(std::filesystem::exists(this->m_tempDir / ".camko"));
	EXPECT_TRUE(std::filesystem::is_directory(this->m_tempDir / ".camko"));
	EXPECT_TRUE(std::filesystem::exists(this->m_tempDir / "include"));
	EXPECT_TRUE(std::filesystem::is_directory(this->m_tempDir / "include"));
	EXPECT_TRUE(std::filesystem::exists(this->m_tempDir / "src"));
	EXPECT_TRUE(std::filesystem::is_directory(this->m_tempDir / "src"));
	EXPECT_TRUE(std::filesystem::exists(this->m_tempDir / "config.toml"));
	EXPECT_TRUE(std::filesystem::exists(this->m_tempDir / "src" / "main.cpp"));
}

TEST_F(InitCommandTest, NoArgs_ConfigTomlContentMatchesDefault)
{
	CommandError result = this->m_command.Execute({});
	ASSERT_TRUE(result.valid);

	std::string content = Marco::ReadFile(this->m_tempDir / "config.toml");
	EXPECT_EQ(content, defaults::kDefaultConfigToml);
}

TEST_F(InitCommandTest, NoArgs_mainCppContentatchesDefault)
{
	CommandError result = this->m_command.Execute({});
	ASSERT_TRUE(result.valid);

	std::string content = Marco::ReadFile(this->m_tempDir / "src" / "main.cpp");
	EXPECT_EQ(content, defaults::kDefaultMainCpp);
}

TEST_F(InitCommandTest, WithProjectNameArg_CreatesProjectInNamedSubdirectory)
{
	CommandError result = this->m_command.Execute({"my_project"});

	std::filesystem::path projectPath = this->m_tempDir / "my_project";
	EXPECT_TRUE(std::filesystem::exists(projectPath / ".camko"));
	EXPECT_TRUE(std::filesystem::exists(projectPath / "include"));
	EXPECT_TRUE(std::filesystem::exists(projectPath / "src"));
	EXPECT_TRUE(std::filesystem::exists(projectPath / "config.toml"));
	EXPECT_TRUE(std::filesystem::exists(projectPath / "src" / "main.cpp"));

	EXPECT_FALSE(std::filesystem::exists(this->m_tempDir / ".camko"));
}

TEST_F(InitCommandTest, TooManyArgs_ReturnsErrorAndCreatesNothing)
{
	CommandError result = this->m_command.Execute({"one", "two"});

	EXPECT_FALSE(result.valid);
	EXPECT_EQ(result.message, "Too many arguments");

	EXPECT_FALSE(std::filesystem::exists(this->m_tempDir / ".camko"));
	EXPECT_FALSE(std::filesystem::exists(this->m_tempDir / "config.toml"));
}

TEST_F(InitCommandTest, RunningTwiceInSameDirectory_ReturnsError)
{
	CommandError result = this->m_command.Execute({});
	ASSERT_TRUE(result.valid);

	result = this->m_command.Execute({});
	EXPECT_FALSE(result.valid);
}

TEST_F(InitCommandTest, RunningTwiceWithNamedProject_ReturnsError)
{
	CommandError result = this->m_command.Execute({"my_project"});
	ASSERT_TRUE(result.valid);
	
	result = this->m_command.Execute({"my_project"});
	EXPECT_FALSE(result.valid);
}