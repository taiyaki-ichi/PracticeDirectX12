#pragma once
#include"resource.hpp"
#include"../Utility.hpp"
#include<type_traits>
#include<iterator>

namespace DX12
{
	template<typename Resource, typename T>
	inline void map_impl(Resource* resource, T&& t)
	{
		using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
		ValueType* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);
		*target = std::forward<T>(t);
		resource->get()->Unmap(0, nullptr);
	}

	template<typename Resource,typename Iter>
	inline void map_impl(Resource* resource, Iter first, Iter last)
	{
		using ValueType = typename std::iterator_traits<Iter>::value_type;
		ValueType* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);
		std::copy(first, last, target);
		resource->get()->Unmap(0, nullptr);
	}

	template<typename Resource,typename T>
	inline void map_impl(Resource* resource, T* data, std::uint32_t width, std::uint32_t height,std::uint32_t targetPitch)
	{
		using value_type = std::remove_cv_t<T>;
		value_type* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);

		for (std::uint32_t i = 0; i < height; i++)
		{
			std::copy_n(data, width, target);
			data += width;
			target += targetPitch;
		}

		resource->get()->Unmap(0, nullptr);
	}

	template<typename Resource,typename... Ts>
	inline void map(Resource* resource, Ts&&... ts)
	{
		static_assert((Resource::flags | static_cast<D3D12_HEAP_TYPE>(resource_heap_property::Upload)) ||
			Resource::flags | static_cast<D3D12_HEAP_TYPE>(resource_heap_property::ReadBack));

		map_impl(resource, std::forward<Ts>(ts)...);
	}



	


}