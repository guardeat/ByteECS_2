#ifndef BYTE_SHRINKVECTOR_H
#define BYTE_SHRINKVECTOR_H

#include <vector>

namespace Byte
{

	template<typename T, typename Allocator = std::allocator<T>>
	class shrink_vector: public std::vector<T,Allocator>
	{
	private:
		inline static constexpr float MIN_LOAD{ 0.25 };

	public:
		void pop_back()
		{
			std::vector<T, Allocator>::pop_back();
			check_shrink();
		}

		void erase()
		{
			std::vector<T, Allocator>::erase();
			check_shrink();
		}

	private:
		void check_shrink()
		{
			if (this->size() / static_cast<float>(this->capacity()) < MIN_LOAD)
			{
				this->shrink_to_fit();
			}
		}
	};

}

#endif
