#ifndef BYTE_LOGGER_H
#define BYTE_LOGGER_H

#include <iostream>

namespace Byte
{

	struct Logger
	{
		Logger() = delete;

		Logger(const Logger&) = delete;

		Logger(Logger&&) = delete;

		template<typename Type, typename... Types>
		static void log(const Type& arg, const Types&... args)
		{
			std::cout << (arg);
			((std::cout << " " << (args)), ...);
			std::cout << std::endl;
		}

		static void log()
		{
			std::cout << std::endl;
		}

	};

}

#endif