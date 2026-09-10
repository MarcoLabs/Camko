#include "Defaults.h"
#include <string_view>

namespace defaults
{
	const std::string_view kDefaultConfigToml = R"toml(
[project]
name = "main"
version = "0.1.0"

[build]
type = "Debug" # Debug | Release | RelWithDebInfo | MinSizeRel
cpp-version = 23
source-directory = "src"
header-directory = "include"
build-shared-libs = false
enable-warnings = true
warnings-as-errors = false
enable-sanitizers = false  # Build with ASan/UBSan enabled
enable-lto = false         # Enable link-time optimization
enable-ccache = true       # Enable ccache when available

[tests]
enable-tests = false
tests-directory = "tests"

# [[dependencies]]
# name = "fmt"
# repo = "https://github.com/fmtlib/fmt.git"
# version = "10.2.1"
# link-target = "fmt::fmt" # if omitted, automatically becomse [name]::[name]

# [[dependencies]]
# name = "json"
# repo = "https://github.com/nlohmann/json.git"
# version = "v3.11.3"
# link-target = "nlohmann_json::nlohmann_json"
# find-package-name = "nlohmann_json" # only needed if installed using a package manager
)toml";

	const std::string_view kDefaultMainCpp = R"cpp(
#include <iostream>

int main()
{
	std::cout << "Hello World!" << std::endl;

	return 0;
}
	)cpp";
}