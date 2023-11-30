#ifndef BYTE_ECS_COMPONENT_H
#define	BYTE_ECS_COMPONENT_H

#include <memory>
#include <vector>

#include "typedefs.h"

namespace Byte::ECS
{

	struct ComponentIDGenerator
	{
	private:
		inline static ComponentID next{ 0 };

	public:
		template<typename Component>
		static ComponentID generate()
		{
			return next++;
		}
	};

	template<typename Component>
	class ComponentRegistry
	{	
	private:
		inline static const ComponentID ID{ ComponentIDGenerator::generate<Component>() };

	public:
		static ComponentID getID()
		{
			return ID;
		}
	};

}

#endif