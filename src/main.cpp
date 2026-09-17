#include "CommandRegistry.h"
#include "Defaults.h"
#include <print>
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

	if (! commands.Find(argv[1]))
	{
		std::println("Unknown command '{}'", argv[1]);

		return 1;
	}

	Command* command = commands.Find(argv[1]);

	const auto trimmedArgv = TrimArgvFromFirstTwoElements(argc, argv);

	CommandError error = command->Run(trimmedArgv);

	if (! error.valid)
	{
		std::println("{}", error.message);

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
		std::println("{}", defaults::kDefaultHelpMessage);

		return true;
	}

	std::string_view arg = argv[1];

	if (arg == "-v" || arg == "--version")
	{
		std::println("{}", defaults::kCamkoVersion);

		return true;
	}
	else if (arg == "-h" || arg == "--help")
	{
		std::println("{}", defaults::kDefaultHelpMessage);

		return true;
	}

	return false;
}
