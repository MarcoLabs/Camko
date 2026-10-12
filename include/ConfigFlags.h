#pragma once

#include "CommandError.h"
#include "Config.h"
#include <string>
#include <vector>

namespace ConfigFlags
{
	CommandError Apply(Config* config, const std::vector<std::string>& flags);
}
