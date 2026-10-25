#include "CommandRegistry.h"
#include "Config.h"
#include "Defaults.h"
#include "Utils/FlagUtils.h"
#include "Utils/General.h"
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

static std::pair<std::vector<std::string>, std::vector<std::string>> SplitArgvIntoFlagsAndRunArguments(int argc, const char** argv); // first element is always camko and second is the option name
static bool TryHandleGlobalFlags(int argc, const char** argv);

int main(int argc, const char** argv)
{
	if (TryHandleGlobalFlags(argc, argv))
	{
		return 0;
	}

	const auto& commands = CommandRegistry::Instance();
	Config& config       = Config::Instance();

	if (!commands.Find(argv[1]))
	{
		std::cout << "Unknown command '" << argv[1] << "'" << std::endl;

		return 1;
	}

	auto projectRoot = utils::GetCamkoProjectRootDirectory();
	if (! projectRoot)
	{
		if (std::strcmp(argv[1], "init") == 0)
		{
			CommandError result = config.Parse(defaults::kDefaultConfigToml);

			if (! result.valid)
			{
				std::unreachable();
			}
		}
		else
		{
			std::cout << projectRoot.error().message << std::endl;

			return 1;
		}

	}
	else
	{
		auto result = config.Parse(*projectRoot / defaults::kConfigFileName);
		if (! result.valid)
		{
			std::cout << "Error in the config file: " << result.message << std::endl;

			return 1;
		}
	}
	auto [commandFlags, runArguments] = SplitArgvIntoFlagsAndRunArguments(argc, argv);

	CommandError error = config.FillWithFlags(commandFlags);
	if (! error.valid)
	{
		std::cout << error.message << std::endl;

		return 1;
	}

	Command* command = commands.Find(argv[1]);

	error = command->Run(config, runArguments);

	if (!error.valid)
	{
		std::cout << error.message << std::endl;

		return 1;
	}

	return 0;
}

std::pair<std::vector<std::string>, std::vector<std::string>> SplitArgvIntoFlagsAndRunArguments(int argc, const char** argv)
{
	std::vector<std::string> commandFlags{};
	std::vector<std::string> runArguments{};

	int i = 2;
	for (; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--") == 0)
		{
			break;
		}

		if (Utils::HasFlagPrefix(argv[i]))
		{
			commandFlags.push_back(argv[i]);
		}
		else
		{
			runArguments.push_back(argv[i]);
		}
		
	}

	i++; // skip the potential "--"
	for (; i < argc; i++)
	{
		runArguments.push_back(argv[i]);
	}

	return {commandFlags, runArguments};
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
