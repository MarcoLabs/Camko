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
warnings-as-errors = false # Enable ASan/UBSan
enable-lto = false         # Enable link-time optimization
enable-ccache = true       # Enable ccache when available

[tests]
enable-tests = false
tests-directory = "tests"

[examples]
enable-examples = false
example-directory = "examples"
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