#ifndef BYTE_ECS_CLUSTER_H
#define BYTE_ECS_CLUSTER_H

#include "signature.h"
#include "accessor.h"
#include "shrink_vector.h"
#include "component.h"

#include <unordered_map>
#include <memory>

namespace Byte::ECS
{

	class Cluster
	{
	private:
		using EntityIDContainer = shrink_vector<EntityID>;
		using AccessorMap = std::unordered_map<ComponentID,UniqueAccessor>;

		friend struct ClusterBuilder;
		friend struct ClusterBridge;
		template<typename... Types>
		friend struct ClusterCache;

	private:
		Signature _signature;
		EntityIDContainer _entities;
		AccessorMap accessors;

	public:
		Cluster() = default;

		Cluster(const Signature& _signature)
			:_signature{ _signature }
		{
		}

		Cluster(const Cluster& left)
			:Cluster{ copy() }
		{
		}

		Cluster(Cluster&& right) noexcept = default;

		Cluster& operator=(const Cluster& left)
		{
			*this = left.copy();
			return *this;
		}

		Cluster& operator=(Cluster&& right) noexcept
		{
			_signature = right._signature;
			_entities = std::move(right._entities);
			accessors = std::move(right.accessors);

			right._signature.clear();

			return *this;
		}

		const Signature& signature() const
		{
			return _signature;
		}

		const EntityIDContainer& entities() const
		{
			return _entities;
		}

		void pushEntity(EntityID id)
		{
			_entities.push_back(id);
		}

		EntityID remove(size_t index)
		{
			EntityID out{ _entities[size() - 1] };
			
			_entities[index] = out;
			
			for (auto& pair : accessors)
			{
				pair.second->swap(index, size() - 1);
				pair.second->pop();
			}

			_entities.pop_back();

			return out;
		}

		template<typename Type>
		void push(Type&& item)
		{
			accessor<Type>().push(std::move(item));
		}

		template<typename Type, typename... Args>
		void emplace(Args&&... items)
		{
			accessor<Type>().emplace(std::move(items));
		}

		template<typename Type>
		Type& get(size_t index)
		{
			return accessor<Type>().at(index);
		}

		template<typename Type>
		const Type& get(size_t index) const
		{
			return accessor<Type>().at(index);
		}

		size_t size() const
		{
			return _entities.size();
		}

		bool empty() const
		{
			return size() == 0;
		}

		Cluster copy() const
		{
			Cluster out{ _signature };
			out._entities = _entities;
			for (auto& pair : accessors)
			{
				out.accessors[pair.first] = pair.second->copy();
			}
			return out;
		}

	private:
		template<typename Type>
		Accessor<Type>& accessor()
		{
			ComponentID id{ ComponentRegistry<Type>::id };
			return static_cast<Accessor<Type>&>(*accessors.at(id));
		}
		
		template<typename Type>
		const Accessor<Type>& accessor() const
		{
			ComponentID id{ ComponentRegistry<Type>::id };
			return static_cast<Accessor<Type>&>(*accessors.at(id));
		}
	};

	struct ClusterBuilder
	{
		template<typename... Types>
		static Cluster build()
		{
			Cluster out{ SignatureBuilder<Types...>()};
			(add<Types>(out), ...);
			return out;
		}

		template<typename... Types>
		static Cluster build(const Cluster& initial)
		{
			Signature signature{ initial._signature };
			Cluster out{ signature };

			for (auto& pair : initial.accessors)
			{
				out.accessors[pair.first] = pair.second->instance();
			}

			((add<Types>(out), out._signature.set(ComponentRegistry<Types>::id)), ...);
			return out;
		}

		template<typename Type>
		static Cluster buildWithout(const Cluster& initial)
		{
			Signature signature{ initial._signature };
			signature.set(ComponentRegistry<Type>::id, false);
			Cluster out{ signature };

			for (auto& pair : initial.accessors)
			{
				if (pair.first != ComponentRegistry<Type>::id)
				{
					out.accessors[pair.first] = pair.second->instance();
				}
			}

			return out;
		}

	private:
		template<typename Type>
		static void add(Cluster& cluster)
		{
			cluster.accessors[ComponentRegistry<Type>::id] = std::make_unique<Accessor<Type>>();
		}
	};

	struct ClusterBridge
	{
		static size_t carry(Cluster& source, Cluster& destination, EntityID id, size_t index)
		{
			destination.pushEntity(id);
			for (auto& pair : source.accessors)
			{
				auto accessor{destination.accessors.find(pair.first)};
				if (accessor!= destination.accessors.end())
				{
					accessor->second->carryIn(pair.second, index);
				}
			}
			return destination.size() - 1;
		}

		static size_t copy(const Cluster& source, Cluster& destination, EntityID id, size_t index)
		{
			destination.pushEntity(id);
			for (auto& pair : source.accessors)
			{
				auto accessor{ destination.accessors.find(pair.first) };
				if (accessor != destination.accessors.end())
				{
					accessor->second->copyIn(pair.second, index);
				}
			}
			return destination.size() - 1;
		}
	};

	template<typename... Types>
	using IDComponentGroup = std::tuple<EntityID, Types&...>;
	template<typename... Types>
	using ComponentGroup = std::tuple<Types&...>;

	template<typename... Types>
	struct ClusterCache
	{
	private:
		using EntityIDContainer = typename Cluster::EntityIDContainer;
		using AccessorMap = typename Cluster::AccessorMap;
		using AccessorCache = std::vector<IAccessor*>;
		using IDComponentGroup = IDComponentGroup<Types&...>;
		using ComponentGroup = ComponentGroup<Types&...>;

	private:
		AccessorCache accessors;
		EntityIDContainer* entities{ nullptr };
		size_t accessorCount{ 0 };

	public:
		ClusterCache(Cluster& cluster)
			:entities{ &cluster._entities }, accessorCount{ cluster.accessors.size() }
		{
			((accessors.push_back(cluster.accessors.at(ComponentRegistry<Types>::id).get())),...);
		}

		ClusterCache() = default;

		IDComponentGroup groupWithID(size_t index)
		{
			size_t iterator{ accessorCount };
			return IDComponentGroup(entities->at(index), get<Types>(index, --iterator)...);
		}

		ComponentGroup group(size_t index)
		{
			size_t iterator{ accessorCount };
			return ComponentGroup(get<Types>(index, --iterator)...);
		}

		size_t size() const
		{
			if (accessors.empty())
			{
				return 0;
			}
			return accessors[0]->size();
		}

	private:
		template<typename Type>
		Type& get(size_t index, size_t accessorIndex)
		{
			return static_cast<Accessor<Type>*>(accessors[accessorIndex])->at(index);
		}

	};

	using ClusterContainer = std::unordered_map<Signature, Cluster>;
	using ClusterGroup = std::vector<Cluster*>;

}

#endif