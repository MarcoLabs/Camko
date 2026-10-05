#pragma once

#include "Command.h"
#include "CommandError.h"
#include <filesystem>
#include <string>
#include <vector>

class BuildCommand : public Command
{
public:
	CommandError Execute(const Config& config, const std::vector<std::string>& args) override;
	std::string Name() const override;

	static CommandError BuildProject     (const Config& config, const std::filesystem::path& projectRoot);
	
	static CommandError ConfigureProject (const Config& config, const std::filesystem::path& camkoDir, const std::filesystem::path& buildDir);
	static void         BuildCmakeProject(const std::filesystem::path& buildDir);

	static void ConstructCMakeLists(const std::filesystem::path& projectRoot, const Config& config);
private:
	
	static std::string ConstructCMakeProjectDefinition (const Config& config);
	static std::string ConstructCMakeLanguageStandard  (const Config& config);
	static std::string ConstructUserConfigurableOptions(const Config& config);
	static std::string ConstructTestOptions            (const Config& config);
	static std::string ConstructExamplesOptions        (const Config& config);
	static std::string ConstructDependencies           (const Config& config);

	static std::string ConstructBuildType              ();
	static std::string ConstructTooling                ();
	static std::string ConstructPositionIndependentCode();
	static std::string ConstructCompilerWarnings       ();
	static std::string ConstructSantitizers            ();
	static std::string ConstructSourceFiles            ();
	static std::string ConstructTesting                ();
	static std::string ConstructExamples               ();
	static std::string ConstructInstallRules           ();

};
