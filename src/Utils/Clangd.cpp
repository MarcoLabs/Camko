#include "Utils/Clangd.h"
#include "Defaults.h"
#include <fstream>

CommandError utils::CreateClangdFile(const std::filesystem::path &projectPath, const std::string &buildSystem)
{
	std::ofstream clangdFile(projectPath / ".clangd");
	if (! clangdFile)
	{
		return CommandError{false, "Could not create .clangd file"};
	}

	clangdFile << defaults::kClangFile << '/' << buildSystem << '\n';
	clangdFile.close();

	return CommandError{true, "No errors occured"};
}
