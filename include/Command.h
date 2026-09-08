#pragma once

#include <string>
#include <vector>
class Command
{
public:
	virtual ~Command() = default;
	virtual int execute(const std::vector<std::string>& args) = 0;
	virtual std::string name() const = 0;
};
