#ifndef	BYTE_TIMER_H
#define BYTE_TIMER_H

#include <chrono>

namespace Byte
{

	using Second = double;
	using Millisecond = double;
	using Nanosecond = size_t;

	class Timer
	{
	private:
		using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	private:
		TimePoint startTime;

	public:
		Timer()
		{
			reset();
		}

		Second seconds() const
		{
			return milliseconds() / 1000.0;
		}

		Millisecond milliseconds() const
		{
			return static_cast<double>(nanoseconds() / 1000000.0);
		}

		Nanosecond nanoseconds() const
		{
			auto duration = std::chrono::high_resolution_clock::now() - startTime;
			return static_cast<size_t>(duration.count());
		}

		void reset()
		{
			startTime = std::chrono::high_resolution_clock::now();
		}
	};

}

#endif
