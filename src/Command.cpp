#include "Command.h"
#include "CommandError.h"
#include "CommandRegistry.h"

std::expected<std::string, CommandError> Command::GetProjectName(const Marco::Toml& toml)
{
	auto projectSettings = toml["project"];
	if (! projectSettings)
	{
		return std::unexpected(CommandError{false, "Could not find the project table in config.toml"});
	}

	auto projectName = (*projectSettings).get()["name"];
	if (! projectName || ! projectName.value().get().IsString())
	{
		return std::unexpected(CommandError{false, "The name variable in config.toml was not set or is not a string"});
	}

	return (*projectName).get().AsString()->get();
}

CommandError Command::Run(const std::vector<std::string>& args)
{
	CommandError error{};
	
	if (args.size() >= 1 && (args[0] == "--help" || args[0] == "-h"))
	{
		error = this->PrintHelp();

		return error;
	}

	error = this->Execute(args);
	
	return error;
}

CommandError Command::PrintHelp()
{
	const auto& commands = CommandRegistry::Instance();

	CommandError error = commands.Find("help")->Execute({ this->Name() });

	return error;
}