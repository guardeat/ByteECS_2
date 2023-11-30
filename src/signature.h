#ifndef BYTE_ECS_SIGNATURE_H
#define BYTE_ECS_SIGNATURE_H

#include <cstdint>
#include <xhash>
#include <vector>
#include <bit>
#include <algorithm>

#include "component.h"
#include "typedefs.h"

namespace Byte::ECS
{
	
	class Signature
	{
	public:
		inline static constexpr size_t BITSET_COUNT{ (MAX_COMPONENT_COUNT/64) + 1 };
		inline static constexpr size_t BIT_COUNT{ 64 };

	private:
		using Container = size_t[BITSET_COUNT];

		Container data{ 0 };

	public:
		Signature() = default;

		void set(ComponentID id)
		{
			size_t bitIndex{ id % BIT_COUNT };
			data[id / BIT_COUNT] |= 1ULL << bitIndex;
		}

		void set(ComponentID id, bool value)
		{
			size_t bitIndex{ id % BIT_COUNT };
			if (value)
			{
				data[id / BIT_COUNT] |= 1ULL << bitIndex;
			}
			else
			{
				data[id / BIT_COUNT] &= ~(1ULL << bitIndex);
			}
		}

		bool test(ComponentID id) const
		{
			return (data[id / BIT_COUNT]) & (1ULL << (id % BIT_COUNT));
		}

		bool include(const Signature& signature) const
		{
			for (size_t index{}; index < BITSET_COUNT; ++index)
			{
				if ((data[index] | signature.data[index]) != data[index])
				{
					return false;
				}
			}
			return true;
		}

		bool match(const Signature& signature) const
		{
			for (size_t index{}; index < BITSET_COUNT; ++index)
			{
				if ((data[index] & signature.data[index]) > 0)
				{
					return true;
				}
			}
			return false;
		}

		bool any() const
		{
			for (size_t index{}; index < BITSET_COUNT; ++index)
			{
				if (data[index] > 0)
				{
					return true;
				}
			}
			return false;
		}

		bool none() const
		{
			return !any();
		}

		void clear()
		{
			std::fill_n(data, BITSET_COUNT, 0);
		}

		bool operator==(const Signature& left) const
		{
			for (size_t index{}; index < BITSET_COUNT; ++index)
			{
				if (data[index] != left.data[index])
				{
					return false;
				}
			}
			return true;
		}

		bool operator!=(const Signature& left) const
		{
			return !(*this == left);
		}
	};

	template<typename... Args>
	struct SignatureBuilder
	{
		operator Signature() const
		{
			Signature out;
			(out.set(ComponentRegistry<Args>::getID()),...);
			return out;
		}
	};

}

namespace std
{
	
	template<>
	struct hash<Byte::ECS::Signature>
	{
		size_t operator()(const Byte::ECS::Signature& signature)const
		{
			size_t result{ 0 };

			return result;
		}
	};

}

#endif
