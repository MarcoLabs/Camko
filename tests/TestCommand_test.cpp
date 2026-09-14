#include "TestCommand.h"
#include <gtest/gtest.h>

TEST(TestCommandName, NameReturnsTest)
{
	TestCommand command{};

	EXPECT_EQ(command.Name(), "test");
}