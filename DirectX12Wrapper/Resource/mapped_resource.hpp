#pragma once
#include"../format.hpp"

namespace DX12
{

	//èâä˙âªçœÇ›ÇÃmapped_resourceÇï‘Ç∑
	//ResourceÇÃmapped_resource_typeÇégÇ¢å^Çé©ìÆìIÇ…ì±Ç≠óp
	template<typename Resource>
	inline auto map(Resource&)->typename Resource::mapped_resource_type;

	
	template<typename FormatTuple>
	class formats_mapped_resource
	{
		void* ptr;

	public:
		template<typename Resource>
		void initialize(Resource&);

		template<std::size_t FormatIndex>
		auto reference(std::size_t num, std::size_t elementNum)
			->typename FormatTuple::template value_type<FormatIndex>&;
	};

	template<typename T>
	class struct_mapped_resource
	{
		void* ptr;
	public:
		template<typename Resource>
		void initialize(Resource&);

		T& reference();
	};

	template<typename Format>
	class texture_upload_mapped_resource
	{
		void* ptr;
		std::uint32_t width = 0;
	public:
		template<typename Resource>
		void initialize(Resource&);

		auto reference(std::uint32_t x,std::uint32_t y, std::uint32_t elementNum)
			-> typename convert_type<Format::component_type, Format::component_size>::type&;
	};


	//
	//
	//

	
	template<typename Resource>
	auto map(Resource& r) -> typename Resource::mapped_resource_type
	{
		using MappedResource = typename Resource::mapped_resource_type;
		MappedResource result{};
		result.initialize(r);
		return result;
	}
	
	

	template<typename FormatTuple>
	template<typename Resource>
	inline void formats_mapped_resource<FormatTuple>::initialize(Resource& r)
	{
		if(FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");
	}

	template<typename FormatTuple>
	template<std::size_t FormatIndex>
	inline auto formats_mapped_resource<FormatTuple>::reference(std::size_t num, std::size_t elementNum) -> typename FormatTuple::template value_type<FormatIndex>&
	{
		using ValueType = typename FormatTuple::template value_type<FormatIndex>;
		constexpr auto formatStride = FormatTuple::get_formats_stride();
		constexpr auto formatStrideToIndex = FormatTuple::template get_formats_stride_to_index<FormatIndex>();

		auto tmpPtr = static_cast<char*>(ptr);
		tmpPtr += formatStride * num + formatStrideToIndex + sizeof(ValueType) * elementNum;

		return *reinterpret_cast<ValueType*>(tmpPtr);
	}

	template<typename T>
	template<typename Resource>
	inline void struct_mapped_resource<T>::initialize(Resource& r)
	{
		if (FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");
	}

	template<typename T>
	inline T& struct_mapped_resource<T>::reference()
	{
		return *static_cast<T*>(ptr);
	}


	template<typename Format>
	template<typename Resource>
	inline void texture_upload_mapped_resource<Format>::initialize(Resource& r)
	{
		if (FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");

		width = r.get_width();
	}

	template<typename Format>
	inline auto texture_upload_mapped_resource<Format>::reference(std::uint32_t x, std::uint32_t y, std::uint32_t elementNum) -> typename convert_type<Format::component_type, Format::component_size>::type&
	{
		using ValueType = typename convert_type<Format::component_type, Format::component_size>::type;
		
		auto p = static_cast<ValueType*>(ptr);
		p += (width * y + x) * Format::component_num + elementNum;

		return *p;
	}
}