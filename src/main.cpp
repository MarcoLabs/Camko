#include "CommandRegistry.h"
#include "Defaults.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

static std::vector<std::string> TrimArgvFromFirstTwoElements(int argc, const char** argv); // first element is always camko and second is the option name
static bool TryHandleGlobalFlags(int argc, const char** argv);

int main(int argc, const char** argv)
{
	if (TryHandleGlobalFlags(argc, argv))
	{
		return 0;
	}

	const auto& commands = CommandRegistry::Instance();

	if (!commands.Find(argv[1]))
	{
		std::cout << "Unknown command '" << argv[1] << "'" << std::endl;

		return 1;
	}

	Command* command = commands.Find(argv[1]);

	const auto trimmedArgv = TrimArgvFromFirstTwoElements(argc, argv);

	CommandError error = command->Run(trimmedArgv);

	if (!error.valid)
	{
		std::cout << error.message << std::endl;

		return 1;
	}

	return 0;
}

std::vector<std::string> TrimArgvFromFirstTwoElements(int argc, const char** argv)
{
	std::vector<std::string> trimmedArgv(argv + 2, argv + argc);

	return trimmedArgv;
}

bool TryHandleGlobalFlags(int argc, const char** argv)
{
	if (argc == 1)
	{
		std::cout << defaults::kDefaultHelpMessage << std::endl;

		return true;
	}

	std::string_view arg = argv[1];

	if (arg == "-v" || arg == "--version")
	{
		std::cout << defaults::kCamkoVersion << std::endl;

		return true;
	}
	else if (arg == "-h" || arg == "--help")
	{
		std::cout << defaults::kDefaultHelpMessage << std::endl;

		return true;
	}
	else if (arg == "--regenerate-config")
	{
		CommandError error = utils::FillConfigFile(defaults::kDefaultConfigToml);
		if (error.valid)
		{
			std::cout << "Successfully regenerated the config" << std::endl;
		}
		else
		{
			std::cout << error.message << std::endl;
		}
		
		return true;
	}

	return false;
}