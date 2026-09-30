#pragma once

#include <cstdio>
#include <functional>
#include <utility>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

namespace utils
{
	template<typename Function>
	void SuppressOutput(Function&& function)
	{
		std::fflush(stdout);
		std::fflush(stderr);
		
#ifdef _WIN32
		const int oldOut = _dup(_fileno(stdout));
		const int oldErr = _dup(_fileno(stderr));

		const int null = _open("NUL", _O_WRONLY);

		_dup2(null, _fileno(stdout));
		_dup2(null, _fileno(stderr));

		_close(null);
#else
		const int oldOut = dup(STDOUT_FILENO);
		const int oldErr = dup(STDERR_FILENO);

		const int null = open("/dev/null", O_WRONLY);

		dup2(null, STDOUT_FILENO);
		dup2(null, STDERR_FILENO);

		close(null);
#endif

		try
		{
			std::invoke(std::forward<Function>(function));
		}
		catch(...)
		{
#ifdef _WIN32
			_dup2(oldOut, _fileno(stdout));
			_dup2(oldErr, _fileno(stderr));

			_close(oldOut);
			_close(oldErr);
#else
			dup2(oldOut, STDOUT_FILENO);
			dup2(oldErr, STDERR_FILENO);

			close(oldOut);
			close(oldErr);
#endif
			throw;
		}

#ifdef _WIN32
		_dup2(oldOut, _fileno(stdout));
		_dup2(oldErr, _fileno(stderr));

		_close(oldOut);
		_close(oldErr);
#else
		dup2(oldOut, STDOUT_FILENO);
		dup2(oldErr, STDERR_FILENO);

		close(oldOut);
		close(oldErr);
#endif
	}
}
