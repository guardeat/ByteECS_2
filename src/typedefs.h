#ifndef BYTE_ECS_TYPEDEFS_H
#define BYTE_ECS_TYPEDEFS_H

#include <cstdint>

namespace Byte::ECS
{

	using EntityID = uint64_t;
	using ComponentID = uint32_t;

	inline static constexpr size_t MAX_COMPONENT_COUNT{ 1024 };

}

#endif