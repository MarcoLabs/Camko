#include "Defaults.h"
#include <string_view>

namespace defaults
{
	const std::string_view kDefaultConfigToml = 
R"toml([project]
name = "main"
version = "0.1.0"

[build]
type = "Debug" # Debug | Release | RelWithDebInfo | MinSizeRel
cpp-version = 23
source-directory = "src"
header-directory = "include"
)toml";

	const std::string_view kDefaultMainCpp =
R"cpp(#include <iostream>

int main()
{
	std::cout << "Hello World!" << std::endl;

	return 0;
}
	)cpp";

	const std::string_view kDefaultGitIgnore = 
R"cpp(.camko/)cpp";

	const std::string_view kDefaultHelpMessage = R"(
camko 0.1.0
A CMake build system wrapper and project manager for C++

USAGE:
	camko <COMMAND>

COMMANDS:
	init [directory]    Scaffold a new camko project (defaults to current directory)
	build                Generate CMakeLists.txt and compile the project
	run [args...]        Build the project, then run the executable, forwarding args
	test                 Build the project, then run the test suite
	examples             Build the project, then run every discovered example
	help                 Print this message or the help of the given subcommand

OPTIONS:
	-h, --help       Print help information
	-v, --version    Print version information

Run 'camko help <COMMAND>' for more information on a specific command.
)";

	const std::string_view kCamkoVersion = "0.1.0";
}