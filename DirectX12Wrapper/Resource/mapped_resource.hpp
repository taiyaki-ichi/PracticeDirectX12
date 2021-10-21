#pragma once
#include"../format.hpp"
#include"format_tuple_iterator.hpp"

namespace DX12
{
	template<typename Resource>
	class mapped_resource
	{
		void* ptr = nullptr;
		std::size_t num{};

	public:
		void initialize(Resource& r) {
			if (FAILED(r.get()->Map(0, nullptr, &ptr)))
				THROW_EXCEPTION("");
			num = r.get_num();
		}

		template<std::size_t I=0>
		auto begin() {
			if constexpr (is_format_tuple<typename Resource::value_type>::value) {
				auto tmpPtr = static_cast<char*>(ptr) + typename Resource::value_type::template get_formats_stride_to_index<I>();
				return format_tuple_iterator<I, typename Resource::value_type>{tmpPtr};
			}
			else if constexpr (is_format<typename Resource::value_type>::value)
			{
				using ValueType = typename convert_type<Resource::value_type::component_type, Resource::value_type::component_size>::type;
				if constexpr (Resource::value_type::component_num == 1)
					return static_cast<ValueType*>(ptr);
				else 
					static_cast<ValueType(*)[Resource::value_type::component_num]>(ptr);
			}
			else
				return static_cast<typename Resource::value_type*>(ptr);
		}

		template<std::size_t I=0>
		auto end() {
			return begin<I>() + num;
		}
	};
	
	template<typename Resource>
	inline auto map(Resource& r) {
		mapped_resource<Resource> result{};
		result.initialize(r);
		return result;
	}

}