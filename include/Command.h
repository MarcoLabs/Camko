#pragma once

#include "CommandError.h"
#include "marco/toml/Toml.h"
#include <expected>
#include <string>
#include <vector>

class Command
{
public:
	virtual ~Command() = default;
	
	virtual CommandError Execute(const std::vector<std::string>& args) = 0;
	virtual std::string Name() const = 0;

	virtual CommandError Run(const std::vector<std::string>& args);
	virtual CommandError PrintHelp();

protected:
	static std::expected<std::string, CommandError> GetProjectName(const Marco::Toml& toml);
};
