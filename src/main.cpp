#include "CommandRegistry.h"
#include "InitCommand.h"
#include <cstring>
#include <iostream>
#include <print>
#include <string>
#include <vector>

std::vector<std::string> TrimArgvFromFirstTwoElements(int argc, const char** argv); // first element is always camko and second is the option name

int main(int argc, const char** argv)
{
	if (argc <= 1)
	{
		std::cout << "Enter arguments." << std::endl;

		return 1;
	}

	const auto& commands = CommandRegistry::Instance();

	if (! commands.Find(argv[1]))
	{
		std::println("Unknown command '{}'", argv[1]);
	}

	Command* command = commands.Find(argv[1]);
	
	const auto trimmedArgv = TrimArgvFromFirstTwoElements(argc, argv);

	command->Execute(trimmedArgv);

	return 0;
}

std::vector<std::string> TrimArgvFromFirstTwoElements(int argc, const char** argv)
{
	std::vector<std::string> trimmedArgv(argv + 2, argv + argc);

	return trimmedArgv;
}