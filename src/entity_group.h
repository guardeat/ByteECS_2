#ifndef BYTE_ECS_ENTITYGROUP_H
#define BYTE_ECS_ENTITYGROUP_H

#include <vector>

#include "cluster.h"
#include "query.h"
#include "view.h"
#include "typedefs.h"

namespace Byte::ECS
{
	
	class EntityGroup: public std::vector<EntityID>
	{
	private:
		ClusterGroup group;

	public:
		EntityGroup(const ClusterGroup& clusters)
			: group{ clusters }
		{
			size_t size{ 0 };

			for (auto cluster : clusters)
			{
				size += cluster->size();
			}

			reserve(size);
			for (auto cluster : clusters)
			{
				insert(end(), cluster->entities().begin(), cluster->entities().end());
			}
		}

		template<typename Type, typename... Types>
		EntityGroup include()
		{
			return EntityGroup{ Query::include(group,SignatureBuilder<Type,Types...>{}) };
		}

		template<typename Type, typename... Types>
		EntityGroup exclude()
		{
			return EntityGroup{ Query::exclude(group,SignatureBuilder<Type,Types...>{}) };
		}
	};

}

#endif