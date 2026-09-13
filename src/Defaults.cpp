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
}