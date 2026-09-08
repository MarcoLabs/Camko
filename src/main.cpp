#include "InitCommand.h"
#include <cstring>
#include <iostream>
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

	const auto trimmedArgv = TrimArgvFromFirstTwoElements(argc, argv);

	if (std::strcmp(argv[1], "init" ) == 0)
	{
		InitCommand cmd;

		cmd.Execute(trimmedArgv);
	}

	return 0;
}

std::vector<std::string> TrimArgvFromFirstTwoElements(int argc, const char** argv)
{
	std::vector<std::string> trimmedArgv(argv + 2, argv + argc);

	return trimmedArgv;
}