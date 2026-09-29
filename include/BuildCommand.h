#pragma once

#include "Command.h"
#include "CommandError.h"
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

class BuildCommand : public Command
{
public:
	CommandError Execute(const Config& config, const std::vector<std::string>& args) override;
	std::string Name() const override;

	static CommandError BuildProject     (const Config& config, const std::filesystem::path& projectRoot);
	static CommandError ConfigureProject (const Config& config, const std::filesystem::path& projectRoot);
	static CommandError BuildCmakeProject(const Config& config, const std::filesystem::path& projectRoot);

private:
	static std::expected<std::string, CommandError> ConstructCMakeLists             (const Config& config);
	static std::expected<std::string, CommandError> ConstructCMakeProjectDefinition (const Config& config);
	static std::expected<std::string, CommandError> ConstructCMakeLanguageStandard  (const Config& config);
	static std::expected<std::string, CommandError> ConstructUserConfigurableOptions(const Config& config);
	static std::expected<std::string, CommandError> ConstructTestOptions            (const Config& config);
	static std::expected<std::string, CommandError> ConstructExamplesOptions        (const Config& config);
	static std::expected<std::string, CommandError> ConstructDependencies           (const Config& config);

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
