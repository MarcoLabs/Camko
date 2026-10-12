#pragma once

#include "CommandError.h"
#include "Config.h"
#include <string>
#include <vector>

class Command
{
public:
	virtual ~Command() = default;
	
	virtual CommandError Execute(const Config& config, const std::vector<std::string>& args, bool ranWithCustomFlags) = 0;
	virtual std::string Name() const = 0;

	virtual CommandError Run(const Config& config, const std::vector<std::string>& args, bool ranWithCustomFlags = false);
	virtual CommandError PrintHelp(const Config& config);
};
