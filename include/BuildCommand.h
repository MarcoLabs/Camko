#pragma once

#include "Command.h"
#include "CommandError.h"
#include "marco/toml/Toml.h"
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

class BuildCommand : public Command
{
public:
	CommandError Execute(const Config& config, const std::vector<std::string>& args) override;
	std::string Name() const override;

	static CommandError BuildProject     (const Marco::Toml& toml, const std::filesystem::path& projectRoot);
	static CommandError ConfigureProject (const Marco::Toml& toml, const std::filesystem::path& projectRoot);
	static CommandError BuildCmakeProject(const Marco::Toml& toml, const std::filesystem::path& projectRoot);

private:
	static std::expected<std::string, CommandError> ConstructCMakeLists             (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructCMakeProjectDefinition (const Marco::Toml& toml, const std::string& projectName);
	static std::expected<std::string, CommandError> ConstructCMakeLanguageStandard  (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructUserConfigurableOptions(const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructTestOptions            (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructExamplesOptions        (const Marco::Toml& toml);
	static std::expected<std::string, CommandError> ConstructDependencies           (const Marco::Toml& toml);

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
