#ifndef BYTE_ECS_POOL_H
#define BYTE_ECS_POOL_H

#include <unordered_map>
#include <tuple>

#include "cluster.h"
#include "entity_container.h"
#include "signature.h"
#include "entity_group.h"
#include "view.h"
#include "typedefs.h"

namespace Byte::ECS
{

	class Pool
	{
	private:
		ClusterContainer clusters;
		EntityContainer _entities;

	public:
		Pool() = default;

		EntityID create()
		{
			return _entities.create();
		}

		template<typename Type, typename... Types>
		EntityID create(Type&& component, Types&&... components)
		{
			EntityID out{ create() };
			attach(out, std::move(component), std::move(components)...);
			return out;
		}

		void destroy(EntityID id)
		{
			Cluster* cluster{ _entities.getCluster(id) };
			if (cluster)
			{
				localDetach(*cluster, id);
			}
			_entities.destroy(id);
		}

		template<typename Type, typename... Types>
		void attach(EntityID id, Type&& component, Types&&... components)
		{
			Signature signature{ SignatureBuilder<Type,Types...>() };

			Cluster* oldCluster{ _entities.getCluster(id) };
			Cluster* newCluster{ nullptr };
			
			auto result{ clusters.find(signature) };
			if (result != clusters.end())
			{
				newCluster = &result->second;
			}
			else
			{
				if (oldCluster)
				{
					clusters[signature] = ClusterBuilder::build<Type,Types...>(*oldCluster);
				}
				else
				{
					clusters[signature] = ClusterBuilder::build<Type,Types...>();
				}
				newCluster = &clusters[signature];
			}

			if (oldCluster)
			{
				ClusterBridge::carry(*oldCluster, *newCluster, id, _entities.getIndex(id));
				localDetach(*oldCluster, id);
			}
			else
			{
				newCluster->pushEntity(id);
			}

			localAttach<Type,Types...>(*newCluster, std::move(component), std::move(components)...);
			_entities.setCluster(id, newCluster);
			_entities.setIndex(id, newCluster->size() - 1);
		}

		template<typename Type>
		void detach(EntityID id)
		{
			Cluster* oldCluster{ _entities.getCluster(id) };

			Signature signature{ oldCluster->signature() };
			signature.set(ComponentRegistry<Type>::getID(), false);

			if (signature.any())
			{
				auto result{ clusters.find(signature) };
				Cluster* newCluster{ &clusters[signature] };

				if (result == clusters.end())
				{
					clusters[signature] = ClusterBuilder::buildWithout<Type>(*oldCluster);
					newCluster = &clusters[signature];
				}
				else
				{
					newCluster = &result->second;
				}
				ClusterBridge::carry(*oldCluster, *newCluster, id, _entities.getIndex(id));
			}
			localDetach(*oldCluster, id);
			if (signature.none())
			{
				_entities.destroy(id);
			}
		}

		template<typename Type>
		Type& get(EntityID id)
		{
			return _entities.getCluster(id)->get<Type>(_entities.getIndex(id));
		}

		template<typename Type>
		const Type& get(EntityID id) const
		{
			return _entities.getCluster(id)->get<Type>(_entities.getIndex(id));
		}

		template<typename Type>
		bool has(EntityID id)
		{
			return _entities.getCluster(id)->signature().test(ComponentRegistry<Type>::getID());
		}

		void clear()
		{
			clusters.clear();
			_entities.clear();
		}

		size_t size() const
		{
			return _entities.size();
		}

		EntityGroup entities()
		{
			return EntityGroup{ Query::include(clusters, Signature{}) };
		}

		template<typename Type, typename... Types>
		EntityGroup entities()
		{
			return EntityGroup{ Query::include(clusters, SignatureBuilder<Type, Types...>{}) };
		}

		template<typename Type, typename... Types>
		View<Type, Types...> components()
		{
			return View<Type, Types...>(Query::include(clusters, SignatureBuilder<Type, Types...>{}));
		}

		template<typename Type, typename... Types>
		IDView<Type, Types...> componentsWithID()
		{
			return IDView<Type, Types...>(Query::include(clusters, SignatureBuilder<Type, Types...>{}));
		}

		template<typename Type, typename... Types, typename Callable>
		void apply(const Callable& callable)
		{
			View<Type, Types...> view{ components<Type,Types...>() };
			for (auto tuple : view)
			{
				std::apply(callable,tuple);
			}
		}

	private:
		void localDetach(Cluster& cluster, EntityID id)
		{
			size_t newIndex{ _entities.getIndex(id) };
			EntityID changed{ cluster.remove(newIndex) };
			if (!cluster.empty())
			{
				_entities.setIndex(changed, newIndex);
				_entities.setCluster(id, nullptr);
			}
		}

		template<typename... Types>
		void localAttach(Cluster& cluster, Types&&... components)
		{
			(cluster.push<Types>(std::move(components)),...);
		}
	};

}

#endif