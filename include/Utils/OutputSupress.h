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
	void SuppressOutput(Function&& function)
	{
		OutputSuppressor suppressor;

		std::invoke(std::forward<Function>(function));
	}
}
