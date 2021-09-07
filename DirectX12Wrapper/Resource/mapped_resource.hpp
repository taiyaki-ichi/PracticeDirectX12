#pragma once
#include"../format.hpp"

namespace DX12
{

	//‰Šú‰»Ï‚İ‚Ìmapped_resource‚ğ•Ô‚·
	//Resource‚Ìmapped_resource_type‚ğg‚¢Œ^‚ğ©“®“I‚É“±‚­—p
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



}