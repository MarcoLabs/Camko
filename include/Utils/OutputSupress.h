#pragma once

#include <functional>
#include <utility>

namespace utils
{
	class OutputSuppressor
	{
	public:
		OutputSuppressor();
		~OutputSuppressor();

		OutputSuppressor(const OutputSuppressor&) = delete;
		OutputSuppressor& operator=(const OutputSuppressor&) = delete;

	private:
		int m_oldOut;
		int m_oldErr;
	};

	template<typename Function>
	decltype(auto) SuppressOutput(Function&& function)
	{
		OutputSuppressor suppressor;

		return std::invoke(std::forward<Function>(function));
	}
}
