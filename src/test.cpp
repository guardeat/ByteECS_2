#include "logger.h"
#include "timer.h"
#include "pool.h"

using namespace Byte;
using namespace Byte::ECS;

int main()
{
	constexpr size_t ENTITY_COUNT{ 1000000 };
	Timer timer;
	Pool pool;

	timer.reset();
	for (size_t i{}; i < ENTITY_COUNT; ++i)
	{
		pool.create<int, float>(1, 1.0f);
	}
	Logger::log(timer.milliseconds());

	size_t sum{ 0 };

	timer.reset();
	for (size_t k{}; k < 1000; ++k)
	{
		for (size_t i{}; i < ENTITY_COUNT; ++i)
		{
			sum += pool.get<float>(i);
		}
	}
	Logger::log(timer.milliseconds() / 1000,"---");

	Logger::log(sum);
}