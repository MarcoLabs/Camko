#include "Utils/FlagUtils.h"

bool Utils::HasFlagPrefix(std::string_view s)
{
	return s.size() > 2 && s.starts_with("--");
}
