#pragma once

#include "CommandError.h"
#include <string>
#include <vector>

class Command
{
public:
	virtual ~Command() = default;
	virtual CommandError Execute(const std::vector<std::string>& args) = 0;
	virtual std::string Name() const = 0;
};
