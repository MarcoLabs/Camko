#include <gtest/gtest.h>
#include "ExamplesCommand.h"

TEST(ExamplesCommandName, NameReturnsExamples)
{
	ExamplesCommand command{};
	
	EXPECT_EQ(command.Name(), "examples");
}