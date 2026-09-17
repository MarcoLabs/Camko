#include "Defaults.h"
#include "HelpCommand.h"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <string>


TEST(HelpCommandName, NameReturnsHelp)
{
	HelpCommand command{};

	EXPECT_EQ(command.Name(), "help");
}

TEST(HelpCommandExecute, SucceedsOnNoArgs)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, PrintsDefaultHelpMessageOnNoArgs)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	command.Execute({});
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_NE(output.find(defaults::kDefaultHelpMessage), std::string::npos);
}

TEST(HelpCommandExecute, FailsOnTooManyArgs)
{
	HelpCommand command{};

	CommandError error = command.Execute({"init", "build"});

	EXPECT_FALSE(error.valid);
	EXPECT_EQ(error.message, "Too many arguments");
}

TEST(HelpCommandExecute, TreatsEmptyStringArgsAsUnknownCommand)
{
	HelpCommand command{};

	CommandError error = command.Execute({""});

	EXPECT_FALSE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnInit)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"init"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnBuild)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"build"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnRun)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"run"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnTest)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"test"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnExamples)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"examples"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, SucceedsOnHelp)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	CommandError error = command.Execute({"help"});
	testing::internal::GetCapturedStdout();

	EXPECT_TRUE(error.valid);
}

TEST(HelpCommandExecute, PrintsUsageLineForInit)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	command.Execute({"init"});
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_NE(output.find("camko init [DIRECTORY]"), std::string::npos);
}

TEST(HelpCommandExecute, PrintsGoogleTestNoteForTest)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	command.Execute({"test"});
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_NE(output.find("Google Test"), std::string::npos);
}

TEST(HelpCommandExecute, PrintsUsagelineForExamples)
{
	HelpCommand command{};

	testing::internal::CaptureStdout();
	command.Execute({"examples"});
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_NE(output.find("camko examples"), std::string::npos);
}
