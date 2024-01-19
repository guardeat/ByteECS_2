#ifndef BYTE_ECS_ENTITYCONTAINER_H
#define	BYTE_ECS_ENTITYCONTAINER_H

#include "sparse_vector.h"
#include "cluster.h"
#include "typedefs.h"

namespace Byte::ECS
{

	class EntityContainer
	{
	private:
		struct EntityData
		{
			Cluster* cluster{ nullptr };
			size_t index;
		};

		using Container = sparse_vector<EntityData>;

	private:
		Container entities;

	public:
		EntityID create()
		{
			return entities.push(EntityData{});
		}

		void destroy(EntityID id)
		{
			entities.erase(id);

			if (entities.size() / static_cast<double>(entities.capacity()) < 0.25)
			{
				entities.shrink_to_fit();
			}
		}

		Cluster* getCluster(EntityID id)
		{
			return entities.at(id).cluster;
		}

		size_t getIndex(EntityID id)
		{
			return entities.at(id).index;
		}

		void setCluster(EntityID id, Cluster* cluster)
		{
			entities.at(id).cluster = cluster;
		}

		void setIndex(EntityID id, size_t index)
		{
			entities.at(id).index = index;
		}

		void clear()
		{
			entities.clear();
		}

		size_t size() const
		{
			return entities.size();
		}
	};

}

#endif