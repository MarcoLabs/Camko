#include "CommandRegistry.h"
#include "BuildCommand.h"
#include "InitCommand.h"
#include <memory>

CommandRegistry& CommandRegistry::Instance()
{
	static CommandRegistry instance;

	return instance;
}

CommandRegistry::CommandRegistry()
{
	this->m_commands.emplace("init", std::make_unique<InitCommand>());
	this->m_commands.emplace("build", std::make_unique<BuildCommand>());
}

Command* CommandRegistry::Find(const std::string& name) const
{
	auto it = this->m_commands.find(name);

	return it != this->m_commands.end() ? it->second.get() : nullptr;
}