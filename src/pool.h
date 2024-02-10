#ifndef BYTE_ECS_POOL_H
#define BYTE_ECS_POOL_H

#include <unordered_map>
#include <tuple>

#include "cluster.h"
#include "signature.h"
#include "entity_group.h"
#include "view.h"
#include "typedefs.h"

#include "Byte/Container/sparse_vector.h"

namespace Byte::ECS
{

	class Pool
	{
	private:
		struct EntityData
		{
			Cluster* cluster{ nullptr };
			size_t index;
		};

		using EntityContainer = sparse_vector<EntityData>;

		ClusterContainer clusters;
		EntityContainer entityContainer;

	public:
		Pool() = default;

		EntityID create()
		{
			return entityContainer.push(EntityData{});
		}

		template<typename Type, typename... Types>
		EntityID create(Type&& component, Types&&... components)
		{
			EntityID out{ create() };
			attach(out, std::move(component), std::move(components)...);
			return out;
		}

		EntityID copy(EntityID source)
		{
			EntityID out{ create() };
			Cluster& cluster{ *entityContainer[source].cluster };
			entityContainer[out].index = ClusterBridge::copy(cluster, cluster, out, entityContainer[source].index);
			entityContainer[out].cluster = &cluster;

			return out;
		}

		void destroy(EntityID id)
		{
			Cluster* cluster{ entityContainer[id].cluster };
			if (cluster)
			{
				_detach(*cluster, id);
			}
			entityContainer.erase(id);

			if (entityContainer.size() / static_cast<double>(entityContainer.capacity()) < 0.25)
			{
				entityContainer.shrink_to_fit();
			}
		}

		template<typename Type, typename... Types>
		void attach(EntityID id, Type&& component, Types&&... components)
		{
			Signature signature{ SignatureBuilder<Type,Types...>() };

			Cluster* oldCluster{ entityContainer[id].cluster };
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
				ClusterBridge::carry(*oldCluster, *newCluster, id, entityContainer[id].index );
				_detach(*oldCluster, id);
			}
			else
			{
				newCluster->pushEntity(id);
			}

			_attach<Type,Types...>(*newCluster, std::move(component), std::move(components)...);

			entityContainer[id].cluster = newCluster;
			entityContainer[id].index = newCluster->size() - 1;
		}

		template<typename Type>
		void detach(EntityID id)
		{
			Cluster* oldCluster{ entityContainer[id].cluster };

			Signature signature{ oldCluster->signature() };
			signature.set(ComponentRegistry<Type>::id, false);

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
				ClusterBridge::carry(*oldCluster, *newCluster, id, entityContainer[id]);
			}
			_detach(*oldCluster, id);
		}

		template<typename Type>
		Type& get(EntityID id)
		{
			return entityContainer[id].cluster->get<Type>(entityContainer[id].index);
		}

		template<typename Type>
		const Type& get(EntityID id) const
		{
			return entityContainer[id].cluster->get<Type>(entityContainer[id].index);
		}

		template<typename Type>
		bool has(EntityID id)
		{
			return entityContainer[id].cluster->signature().test(ComponentRegistry<Type>::id);
		}

		void clear()
		{
			clusters.clear();
			entityContainer.clear();
		}

		size_t size() const
		{
			return entityContainer.size();
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

		bool contains(EntityID id) const
		{
			return entityContainer.test(id);
		}

	private:
		void _detach(Cluster& cluster, EntityID id)
		{
			size_t newIndex{ entityContainer[id].index };
			EntityID changed{ cluster.remove(newIndex) };
			if (!cluster.empty())
			{
				entityContainer[changed].index = newIndex;
				entityContainer[id].cluster = nullptr;
			}
		}

		template<typename... Types>
		void _attach(Cluster& cluster, Types&&... components)
		{
			(cluster.push<Types>(std::move(components)),...);
		}
	};

}

#endif