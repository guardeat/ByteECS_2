#ifndef BYTE_ECS_TYPEDEFS_H
#define BYTE_ECS_TYPEDEFS_H

#include <cstdint>
#include <limits>

namespace Byte::ECS
{

	using EntityID = uint64_t;
	using ComponentID = uint32_t;

	inline constexpr EntityID nullent{ std::numeric_limits<EntityID>::max() };
	inline constexpr size_t MAX_COMPONENT_COUNT{ 1024 };

}

#endif