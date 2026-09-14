#include "RunCommand.h"
#include <gtest/gtest.h>

TEST(RunCommandName, NameReturnsRun)
{
	RunCommand command{};

	EXPECT_EQ(command.Name(), "run");
}