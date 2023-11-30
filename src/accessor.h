#ifndef BYTE_ECS_ACCESSOR_H
#define BYTE_ECS_ACCESSOR_H

#include <memory>
#include <vector>
#include <algorithm>

#include "component.h"
#include "shrink_vector.h"
#include "typedefs.h"

namespace Byte::ECS
{
	
	template<typename Container>
	struct ContainerTraitsBase
	{
		using Value = typename Container::value_type;
		using Iterator = typename Container::iterator;
		using ConstIterator = typename Container::const_iterator;

	public:
		static void push(Container& container, Value&& value)
		{
			container.push_back(std::move(value));
		}

		template<typename... Args>
		static void emplace(Container& container, Args&&... values)
		{
			container.emplace_back(std::move(values...));
		}

		static Value& at(Container& container, size_t index)
		{
			return container.at(index);
		}

		static const Value& at(const Container& container, size_t index)
		{
			return container.at(index);
		}

		static void set(Container& container, size_t index, Value&& value)
		{
			at(container,index) = std::move(value);
		}

		static void pop(Container& container)
		{
			container.pop_back();
		}

		static void carryIn(Container& source, Container& destination, size_t index)
		{
			push(destination, std::move(at(source, index)));
		}

		static void copyIn(const Container& source, Container& destination, size_t index)
		{
			push(destination, Value{ at(source, index) });
		}

		static void swapItems(Container& container, size_t left, size_t right)
		{
			std::swap(at(container, left), at(container, right));
		}

		static size_t size(const Container& container)
		{
			return container.size();
		}
	};

	template<typename Container>
	struct ContainerTraits: public ContainerTraitsBase<Container>
	{
	};

	class IAccessor;
	using UniqueAccessor = std::unique_ptr<IAccessor>;

	class IAccessor
	{
	public:
		virtual ~IAccessor() = default;

		virtual void pop() = 0;

		virtual void swap(size_t left, size_t right) = 0;

		virtual void carryIn(UniqueAccessor& source, size_t index) = 0;

		virtual void copyIn(const UniqueAccessor& source, size_t index) = 0;

		virtual size_t size() const = 0;

		virtual UniqueAccessor instance() const = 0;

		virtual UniqueAccessor copy() const = 0;
	};

	template<typename Type>
	class Accessor: public IAccessor
	{
	private:
		using Container = shrink_vector<Type>;
		using ContainerTraits = ContainerTraits<Container>;
		using Value = typename ContainerTraits::Value;

	private:
		Container container;

	public:
		Value& at(size_t index)
		{
			return ContainerTraits::at(container, index);
		}

		const Value& at(size_t index) const
		{
			return ContainerTraits::at(container, index);
		}

		void push(Value&& item)
		{
			ContainerTraits::push(container, std::move(item));
		}

		template<typename... Args>
		void emplace(Args&&... items)
		{
			ContainerTraits::emplace(container, std::move(items));
		}

		void pop() override
		{
			ContainerTraits::pop(container);
		}

		void swap(size_t left, size_t right) override
		{
			ContainerTraits::swapItems(container, left, right);
		}

		void carryIn(UniqueAccessor& source, size_t index) override
		{
			Accessor& casted{ static_cast<Accessor<Type>&>(*source) };
			ContainerTraits::carryIn(casted.container,container,index);
		}

		void copyIn(const UniqueAccessor& source, size_t index) override
		{
			const Accessor& casted{ static_cast<const Accessor<Type>&>(*source) };
			ContainerTraits::copyIn(casted.container, container, index);
		}

		size_t size() const override
		{
			return ContainerTraits::size(container);
		}

		UniqueAccessor instance() const override
		{
			return std::make_unique<Accessor<Type>>();
		}

		UniqueAccessor copy() const override
		{
			return std::make_unique<Accessor<Type>>(*this);
		}
	};
}

#endif