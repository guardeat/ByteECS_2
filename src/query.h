#ifndef BYTE_ECS_QUERY_H
#define	BYTE_ECS_QUERY_H

#include "cluster.h"

namespace Byte::ECS
{

	struct Query
	{
		static ClusterGroup include(ClusterContainer& clusters, const Signature& signature)
		{
			ClusterGroup out;

			for (auto& pair : clusters)
			{
				if (pair.second.signature().includes(signature) && !pair.second.empty())
				{
					out.push_back(&pair.second);
				}
			}

			return out;
		}

		static ClusterGroup include(ClusterGroup& clusters, const Signature& signature)
		{
			ClusterGroup out;

			for (auto& cluster: clusters)
			{
				if (cluster->signature().includes(signature) && !cluster->empty())
				{
					out.push_back(cluster);
				}
			}

			return out;
		}

		static ClusterGroup exclude(ClusterContainer& clusters, const Signature& signature)
		{
			ClusterGroup out;

			for (auto& pair : clusters)
			{
				if (!pair.second.signature().matches(signature) && !pair.second.empty())
				{
					out.push_back(&pair.second);
				}
			}

			return out;
		}

		static ClusterGroup exclude(ClusterGroup& clusters, const Signature& signature)
		{
			ClusterGroup out;

			for (auto& cluster : clusters)
			{
				if (!cluster->signature().matches(signature) && !cluster->empty())
				{
					out.push_back(cluster);
				}
			}

			return out;
		}
	};

}

#endif