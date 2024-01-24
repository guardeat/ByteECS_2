#ifndef BYTE_ECS_VIEW_H
#define BYTE_ECS_VIEW_H

#include "cluster.h"
#include "typedefs.h"

namespace Byte::ECS
{

	template<typename... Types>
	class _ViewIterator
	{
	protected:
		using Cache = ClusterCache<Types...>;

	protected:
		size_t index;
		ClusterGroup* clusters;
		Cache cache;
		size_t cacheIndex;

	public:
		_ViewIterator(size_t index, ClusterGroup& clusterGroup, size_t cacheIndex)
			:index{ index }, clusters{ &clusterGroup }, cacheIndex{ cacheIndex }
		{
			if (cacheIndex < clusters->size())
			{
				cache = Cache{ *clusters->at(cacheIndex) };
			}
		}

	protected:
		void increment()
		{
			++index;

			if (index == cache.size())
			{
				index = 0;
				++cacheIndex;
				if (cacheIndex != clusters->size())
				{
					cache = Cache{ *clusters->at(cacheIndex) };
				}
			}
		}
	};

	template<typename... Types>
	class ViewIterator : public _ViewIterator<Types...>
	{
	private:
		using ComponentGroup = ComponentGroup<Types...>;

	public:
		ViewIterator(size_t index, ClusterGroup& clusterGroup, size_t cacheIndex)
			:_ViewIterator<Types...>{ index, clusterGroup, cacheIndex }
		{
		}

		ComponentGroup operator*()
		{
			return this->cache.group(this->index);
		}

		ViewIterator& operator++()
		{
			this->increment();
			return *this;
		}

		bool operator==(const ViewIterator& left) const
		{
			return this->cacheIndex == left.cacheIndex;
		}

		bool operator!=(const ViewIterator& left) const
		{
			return !(*this == left);
		}
	};

	template<typename... Types>
	class View
	{
	public:
		using iterator = ViewIterator<Types...>;

	private:
		ClusterGroup clusters;

	public:
		View(const ClusterGroup& clusters)
			: clusters{ clusters }
		{
		}

		template<typename Type, typename... Types>
		View include()
		{
			return View{ Query::include(clusters,SignatureBuilder<Type,Types...>{}) };
		}

		template<typename Type, typename... Types>
		View exclude()
		{
			return View{ Query::exclude(clusters,SignatureBuilder<Type,Types...>{}) };
		}

		iterator begin()
		{
			return iterator{ 0,clusters,0 };
		}

		iterator end()
		{
			return iterator{ 0, clusters, clusters.size() };
		}
	};

	template<typename... Types>
	class IDViewIterator: public _ViewIterator<Types...>
	{
	private:
		using IDComponentGroup = IDComponentGroup<Types...>;

	public:
		IDViewIterator(size_t index, ClusterGroup& clusterGroup, size_t cacheIndex)
			:_ViewIterator<Types...>{index, clusterGroup, cacheIndex}
		{
		}

		IDComponentGroup operator*()
		{
			return this->cache.groupWithID(this->index);
		}

		IDViewIterator& operator++()
		{
			this->increment();
			return *this;
		}

		bool operator==(const IDViewIterator& left) const
		{
			return this->cacheIndex == left.cacheIndex;
		}

		bool operator!=(const IDViewIterator& left) const
		{
			return !(*this == left);
		}
	};

	template<typename... Types>
	class IDView
	{
	public:
		using iterator = IDViewIterator<Types...>;

	private:
		ClusterGroup clusters;

	public:
		IDView(const ClusterGroup& clusters)
			: clusters{clusters}
		{
		}

		template<typename Type, typename... Types>
		IDView include()
		{
			return IDView{ Query::include(clusters,SignatureBuilder<Type,Types...>{}) };
		}

		template<typename Type, typename... Types>
		IDView exclude()
		{
			return IDView{ Query::exclude(clusters,SignatureBuilder<Type,Types...>{}) };
		}

		iterator begin()
		{
			return iterator{ 0,clusters,0 };
		}

		iterator end()
		{
			return iterator{ 0, clusters, clusters.size() };
		}
	};

}

#endif
