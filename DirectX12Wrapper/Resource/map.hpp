#pragma once
#include"resource.hpp"
#include"../utility.hpp"
#include<type_traits>
#include<iterator>
#include<tuple>

namespace DX12
{
	/*
	template<typename Resource, typename T>
	inline void map_impl(Resource* resource, T&& t)
	{
		using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
		ValueType* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);
		*target = std::forward<T>(t);
		resource->get()->Unmap(0, nullptr);
	}

	template<std::size_t I, typename... Args>
	inline void map_element_impl(void* ptr, std::tuple<Args...>& t)
	{
		using value_type = std::remove_reference_t<decltype(std::get<I>(t))>;

		value_type* p = reinterpret_cast<value_type*>(ptr);
		*p = std::get<I>(t);

		if constexpr (I + 1 < sizeof...(Args))
			map_element_impl<I + 1>(reinterpret_cast<void*>(p + 1), t);
	}

	template<typename... Args>
	inline void map_element(void* ptr, std::tuple<Args...>& t)
	{
		map_element_impl<0>(ptr, t);
	}

	template<typename T>
	inline void map_element(void* ptr, T& t)
	{
		static_assert(false);
	}


	template<typename Resource,typename Iter>
	inline void map_impl(Resource* resource, Iter first, Iter last)
	{
		using value_type = typename std::iterator_traits<Iter>::value_type;
		value_type* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);

		if constexpr (std::is_standard_layout_v<value_type>) {
			std::copy(first, last, target);
		}
		else {
			while (first != last) {
				map_element(target, *first);
				first++;
				target++;
			}
		}

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


	template<typename Resource,typename... Args>
	inline void map_impl(Resource* resource, std::tuple<Args...>& t)
	{
		void* target = nullptr;
		resource->get()->Map(0, nullptr, (void**)&target);

		map_element<0>(target, t);

		resource->get()->Unmap(0, nullptr);
	}

	template<typename Resource,typename... Ts>
	inline void map(Resource* resource, Ts&&... ts)
	{
		static_assert((Resource::flags | static_cast<D3D12_HEAP_TYPE>(resource_heap_property::Upload)) ||
			Resource::flags | static_cast<D3D12_HEAP_TYPE>(resource_heap_property::ReadBack));

		map_impl(resource, std::forward<Ts>(ts)...);
	}

	*/
}