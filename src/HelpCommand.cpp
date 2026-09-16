#include "HelpCommand.h"
#include "CommandError.h"
#include "Defaults.h"
#include <print>
#include <string>
#include <vector>

const std::unordered_map<std::string, std::string_view> HelpCommand::helpMessages = {
	{"init", 
R"(
Scaffold a new camko project

USAGE:
camko init [DIRECTORY]

ARGS:
<DIRECTORY>    Where to create the project [default: .]

OPTIONS:
-h, --help    Print help information

This creates the standard project layout: src/, include/, a starter
config.toml, .gitignore and a starter main.cpp.
)"},
	{"build",
R"(
Generate the CMake project and compile it

USAGE:
camko build

OPTIONS:
-h, --help    Print help information

Regenerates .camko/CMakeLists.txt from config.toml and invokes CMake to
compile the project. No flags are currently supported — build behavior
is controlled entirely through config.toml.
)"},
	{"run", 
R"(
Build the project, then run the resulting executable

USAGE:
camko run [ARGS]...

ARGS:
<ARGS>...    Arguments forwarded to the program's main(argc, argv)

OPTIONS:
-h, --help    Print help information

Builds the project first (equivalent to `camko build`), then runs the
resulting executable. Whether anything needs recompiling is left up to
CMake's own incremental build logic.
)"},
	{"test", 
R"(
Build the project, then run the test suite

USAGE:
camko test

OPTIONS:
-h, --help    Print help information

Builds the project and then runs the test suite via CTest. Only files
ending in _test.cpp inside the configured tests directory are picked up,
and only Google Test is supported.

Note: this always attempts to build and run tests, regardless of the
[tests].enable-tests setting in config.toml. If tests-directory is
missing, empty, or has no matching files, expect CMake/CTest to fail
or report no tests found rather than being skipped cleanly.
)"},
	{"examples",
R"(
Build the project, then run every discovered example

USAGE:
camko examples

OPTIONS:
-h, --help    Print help information

Builds the project, then runs every example under examples/. An example
is any subfolder containing its own main.cpp — folders without one are
skipped. Subfolders are searched recursively, so nested examples (e.g.
examples/networking/tcp/main.cpp) are discovered too.

Before each example runs, camko prints its name so output from
different examples doesn't blur together. Examples run one at a time,
in sequence.
)"},
	{"help", 
R"(
Print this message or the help of the given subcommand

USAGE:
camko help [COMMAND]

ARGS:
<COMMAND>    The subcommand to show help for
)"}
};

CommandError HelpCommand::Execute(const std::vector<std::string>& args)
{
	if (args.size() > 1)
	{
		return CommandError{false, "Too many arguments"};
	}
	
	if (args.empty())
	{
		std::println("{}", defaults::kDefaultHelpMessage);

		return CommandError{true, "No errors occured"};
	}

	std::string command = args[0];

	if (helpMessages.find(command) == helpMessages.end())
	{
		return CommandError{false, "Unknown argument. Try camko help [COMMAND] for more information"};
	}

	std::println("{}", helpMessages.at(command));

	return CommandError{true, "No errors occured"};
}

std::string HelpCommand::Name() const
{
	return "help";
}