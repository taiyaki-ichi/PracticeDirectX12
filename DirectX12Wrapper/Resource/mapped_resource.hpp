#pragma once
#include"../format.hpp"

namespace DX12
{

	//初期化済みのmapped_resourceを返す
	//Resourceのmapped_resource_typeを使い型を自動的に導く用
	template<typename Resource>
	inline auto map(Resource&);

	//unmapが必要なのは主にREAD_BACKぐらい
	template<typename Resource>
	inline void unmap(Resource&);

	template<typename FormatTuple>
	struct format_tuple_tag {
		using value_type = FormatTuple;
	};

	template<typename T>
	struct struct_tag {
		using value_type = T;
	};

	template<typename Format>
	struct texture_upload_tag {
		using value_type = Format;
	};


	template<typename FormatTuple>
	class format_tuple_mapped_resource
	{
		void* ptr = nullptr;

	public:
		template<typename Resource>
		void initialize(Resource&);

		template<std::size_t FormatIndex>
		auto reference(std::size_t num, std::size_t elementNum)
			->typename FormatTuple::template value_type<FormatIndex>&;
	};

	template<typename Format>
	class format_mapped_resource
	{
		void* ptr = nullptr;
	public:
		template<typename Resource>
		void initialize(Resource&);

		auto reference(std::size_t num, std::size_t elementNum)
			-> typename convert_type<Format::component_type, Format::component_size>::type&;
	};

	template<typename Format>
	class one_component_format_mapped_resource
	{
		void* ptr = nullptr;
	public:
		template<typename Resource>
		void initialize(Resource&);

		auto reference(std::size_t num)
			-> typename convert_type<Format::component_type, Format::component_size>::type&;
	};

	template<typename T>
	class struct_mapped_resource
	{
		void* ptr = nullptr;
	public:
		template<typename Resource>
		void initialize(Resource&);

		T& reference();
	};

	template<typename Format>
	class texture_upload_mapped_resource
	{
		void* ptr = nullptr;
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
	auto map(Resource& r)
	{
		using Tag = typename Resource::mapped_resource_tag;
		using TagValueType = typename Resource::mapped_resource_tag::value_type;

		auto getMappedResource = []() {
			//テクスチャのマップ用
			if constexpr (std::is_same_v<Tag, texture_upload_tag<TagValueType>>)
				return texture_upload_mapped_resource<TagValueType>{};

			//構造体のマップ用
			if constexpr (std::is_same_v<Tag, struct_tag<TagValueType>>)
				return struct_mapped_resource<TagValueType>{};

			//フォーマットのマップ用
			if constexpr (std::is_same_v<Tag, format_tuple_tag<TagValueType>>)
			{
				if constexpr (TagValueType::get_formats_num() == 1 && TagValueType::template format_type<0>::component_num == 1)
					return one_component_format_mapped_resource<TagValueType::template format_type<0>>{};
				else if constexpr (TagValueType::get_formats_num() == 1)
					return format_mapped_resource<TagValueType::template format_type<0>>{};
				else
					return format_tuple_mapped_resource<TagValueType>{};
			}
		};

		auto result = getMappedResource();
		result.initialize(r);
		return result;
	}
	
	template<typename Resource>
	void unmap(Resource& r)
	{
		r.get()->Unmap(0, nullptr);
	}

	template<typename FormatTuple>
	template<typename Resource>
	inline void format_tuple_mapped_resource<FormatTuple>::initialize(Resource& r)
	{
		if(FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");
	}

	template<typename FormatTuple>
	template<std::size_t FormatIndex>
	inline auto format_tuple_mapped_resource<FormatTuple>::reference(std::size_t num, std::size_t elementNum) -> typename FormatTuple::template value_type<FormatIndex>&
	{
		using ValueType = typename FormatTuple::template value_type<FormatIndex>;
		constexpr auto formatStride = FormatTuple::get_formats_stride();
		constexpr auto formatStrideToIndex = FormatTuple::template get_formats_stride_to_index<FormatIndex>();

		auto tmpPtr = static_cast<char*>(ptr);
		tmpPtr += formatStride * num + formatStrideToIndex + sizeof(ValueType) * elementNum;

		return *reinterpret_cast<ValueType*>(tmpPtr);
	}

	template<typename Format>
	template<typename Resource>
	inline void format_mapped_resource<Format>::initialize(Resource& r)
	{
		if (FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");
	}

	template<typename Format>
	inline auto format_mapped_resource<Format>::reference(std::size_t num, std::size_t elementNum)
		-> typename convert_type<Format::component_type, Format::component_size>::type&
	{
		using ValueType = typename convert_type<Format::component_type, Format::component_size>::type;
		constexpr auto stride = get_format_stride<Format>();

		auto tmpPtr = static_cast<ValueType*>(ptr);
		tmpPtr += Format::component_num * num + elementNum;
		return *tmpPtr;
	}

	template<typename Format>
	template<typename Resource>
	inline void one_component_format_mapped_resource<Format>::initialize(Resource& r)
	{
		if (FAILED(r.get()->Map(0, nullptr, &ptr)))
			THROW_EXCEPTION("");
	}

	template<typename Format>
	inline auto one_component_format_mapped_resource<Format>::reference(std::size_t num)
		-> typename convert_type<Format::component_type, Format::component_size>::type&
	{
		using ValueType = typename convert_type<Format::component_type, Format::component_size>::type;
		auto p = static_cast<ValueType*>(ptr);
		return *(p + num);
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
	inline auto texture_upload_mapped_resource<Format>::reference(std::uint32_t x, std::uint32_t y, std::uint32_t elementNum) 
		-> typename convert_type<Format::component_type, Format::component_size>::type&
	{
		using ValueType = typename convert_type<Format::component_type, Format::component_size>::type;
		
		auto p = static_cast<ValueType*>(ptr);
		p += (width * y + x) * Format::component_num + elementNum;

		return *p;
	}
}