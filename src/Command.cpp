#include "Command.h"
#include "CommandError.h"
#include "CommandRegistry.h"
#include "Config.h"

CommandError Command::Run(const Config& config, const std::vector<std::string>& args)
{
	CommandError error{};
	
	if (args.size() >= 1 && (args[0] == "--help" || args[0] == "-h"))
	{
		error = this->PrintHelp(config);

		return error;
	}

	error = this->Execute(config, args);
	
	return error;
}

CommandError Command::PrintHelp(const Config& config)
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("help")->Execute(config, { this->Name() });

	return error;
}