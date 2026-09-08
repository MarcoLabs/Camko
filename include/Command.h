#pragma once

#include <string>
#include <vector>
class Command
{
public:
	virtual ~Command() = default;
	virtual int Execute(const std::vector<std::string>& args) = 0;
	virtual std::string Name() const = 0;
};
