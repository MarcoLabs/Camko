#pragma once

#include "CommandError.h"
#include <expected>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>

namespace utils
{
	class SuppressOutput
	{
	public:
		template<typename Function>
		SuppressOutput(Function&& function)
		{
			std::ostringstream null;

			auto* oldOut = std::cout.rdbuf(null.rdbuf());
			auto* oldErr = std::cerr.rdbuf(null.rdbuf());

			try
			{
				std::invoke(std::forward<Function>(function));
			}
			catch (...)
			{
				std::cout.rdbuf(oldOut);
				std::cerr.rdbuf(oldErr);
				
				throw;
			}

			std::cout.rdbuf(oldOut);
			std::cerr.rdbuf(oldErr);
		}
	};
	
	std::expected<std::filesystem::path, CommandError> GetCamkoProjectRootDirectory();
	CommandError FillConfigFile(const std::string_view& content); // requires a string with a null terminator
}