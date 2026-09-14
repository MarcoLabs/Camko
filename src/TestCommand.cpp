#include "TestCommand.h"
#include "CommandError.h"
#include <string>
#include <vector>

CommandError TestCommand::Execute(const std::vector<std::string>& args)
{
	
}

std::string TestCommand::Name() const
{
	return "test";
}