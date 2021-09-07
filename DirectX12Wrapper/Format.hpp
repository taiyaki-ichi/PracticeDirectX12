#pragma once
#include"utility.hpp"
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//フォーマットの要素の型用
	enum class component_type
	{
		FLOAT,
		UINT,
		UNSIGNED_NORMALIZE_FLOAT,
		TYPELSEE,
	};

	template<component_type ComponentType, std::uint32_t ComponentSize, std::uint8_t ComponentNum>
	struct format {
		static constexpr component_type component_type = ComponentType;
		static constexpr std::uint32_t component_size = ComponentSize;
		static constexpr std::uint32_t component_num = ComponentNum;
	};

	struct unknow_format {
		//size、numの値はとりあえず
		static constexpr std::uint32_t component_size = 0;
		static constexpr std::uint32_t component_num = 0;
	};

	//formatの用をを実際の型に変換する用
	template<component_type Type, std::size_t Size>
	struct convert_type {
		using type;
	};

	//１format当たりの大きさの取得
	template<typename Format>
	inline constexpr std::uint32_t get_format_stride();

	//indexまでのフォーマットのストライドの合計の取得
	template<std::size_t Index, typename HeadFormat, typename... TaisFormats>
	inline constexpr std::size_t get_formats_stride_to_index();

	//formatを複数個扱う際に使用
	template<typename... Formats>
	struct format_tuple
	{
		template<std::size_t I>
		using format_type = typename index_element<I, Formats...>::type;

		template<std::size_t I>
		using value_type = typename convert_type<format_type<I>::component_type, format_type<I>::component_size>::type;

		template<std::size_t I>
		static constexpr DXGI_FORMAT get_dxgi_format();

		static constexpr std::uint32_t get_formats_stride();
		
		template<std::size_t I>
		static constexpr std::uint32_t get_formats_stride_to_index();

		static constexpr std::uint32_t get_formats_num();
	};


	//0-3bitでnum、4-6でsize、残りがtype
	inline constexpr std::uint32_t get_format_hash(component_type type, std::uint32_t size, std::uint32_t num);
	
	//dxgiのフォーマットの変数の取得
	inline constexpr std::optional<DXGI_FORMAT> get_dxgi_format(component_type type, std::uint32_t size, std::uint32_t num);

	//DepthStencilのDescriptorHeapにViewを作る時に使用
	inline constexpr std::optional<DXGI_FORMAT> get_depth_stencil_dxgi_format(component_type componentType, std::uint8_t componentSize, std::uint8_t componentNum);



	//
	//
	//


	template<>
	struct convert_type<component_type::FLOAT,32> {
		using type = float;
	};

	template<>
	struct convert_type<component_type::UINT, 32> {
		using type = std::uint32_t;
	};

	template<>
	struct convert_type<component_type::UINT, 16> {
		using type = std::uint16_t;
	};

	template<>
	struct convert_type<component_type::UINT, 8> {
		using type = std::uint8_t;
	};


	template<typename Format>
	inline constexpr std::uint32_t get_format_stride()
	{
		return Format::component_size / 8 * Format::component_num;
	}

	template<std::size_t Index, typename HeadFormat, typename... TaisFormats>
	inline constexpr std::size_t get_formats_stride_to_index()
	{
		if constexpr (Index <= 0)
			return 0;
		else
			return get_format_stride<HeadFormat>() + get_formats_stride_to_index<Index - 1, TaisFormats...>();
	}

	template<typename ...Formats>
	template<std::size_t I>
	inline constexpr DXGI_FORMAT format_tuple<Formats...>::get_dxgi_format()
	{
		return DX12::get_dxgi_format(format_type<I>::component_type, format_type<I>::component_size, format_type<I>::component_num).value();
	}


	template<typename ...Formats>
	inline constexpr std::uint32_t format_tuple<Formats...>::get_formats_stride()
	{
		return (get_format_stride<Formats>() + ...);
	}


	template<typename ...Formats>
	template<std::size_t I>
	inline constexpr std::uint32_t format_tuple<Formats...>::get_formats_stride_to_index()
	{
		if constexpr (I <= 0)
			return 0;
		else
			return get_format_stride<format_type<I - 1>>() + get_formats_stride_to_index<I - 2>();
	}

	template<typename ...Formats>
	inline constexpr std::uint32_t format_tuple<Formats...>::get_formats_num()
	{
		return sizeof...(Formats);
	}

	inline constexpr std::uint32_t get_format_hash(component_type type, std::uint32_t size, std::uint32_t num)
	{
		std::uint32_t numHash = 1 << (num - 1);
		std::uint32_t sizeHash = (1 << 4) << ((size / 8) >> 1);
		std::uint32_t typeHash = (1 << 7) << static_cast<std::uint32_t>(type);
		return numHash | sizeHash | typeHash;
	}

	inline constexpr std::optional<DXGI_FORMAT> get_dxgi_format(component_type type, std::uint32_t size, std::uint32_t num)
	{

		switch (get_format_hash(type, size, num))
		{

#define switchCase(t,s,n,v)								\
			case get_format_hash(t,s,n): return v;				\

			switchCase(component_type::FLOAT, 32, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
			switchCase(component_type::FLOAT, 32, 3, DXGI_FORMAT_R32G32B32_FLOAT);
			switchCase(component_type::FLOAT, 32, 2, DXGI_FORMAT_R32G32_FLOAT);
			switchCase(component_type::FLOAT, 32, 1, DXGI_FORMAT_R32_FLOAT);
			switchCase(component_type::FLOAT, 16, 4, DXGI_FORMAT_R16G16B16A16_FLOAT);
			switchCase(component_type::FLOAT, 16, 2, DXGI_FORMAT_R16G16_FLOAT);
			switchCase(component_type::FLOAT, 16, 1, DXGI_FORMAT_R16_FLOAT);
			switchCase(component_type::UINT, 32, 4, DXGI_FORMAT_R32G32B32A32_UINT);
			switchCase(component_type::UINT, 32, 3, DXGI_FORMAT_R32G32B32_UINT);
			switchCase(component_type::UINT, 32, 2, DXGI_FORMAT_R32G32_UINT);
			switchCase(component_type::UINT, 32, 1, DXGI_FORMAT_R32_UINT);
			switchCase(component_type::UINT, 16, 4, DXGI_FORMAT_R16G16B16A16_UINT);
			switchCase(component_type::UINT, 16, 2, DXGI_FORMAT_R16G16_UINT);
			switchCase(component_type::UINT, 16, 1, DXGI_FORMAT_R16_UINT);
			switchCase(component_type::UINT, 8, 4, DXGI_FORMAT_R8G8B8A8_UINT);
			switchCase(component_type::UINT, 8, 2, DXGI_FORMAT_R8G8_UINT);
			switchCase(component_type::UINT, 8, 1, DXGI_FORMAT_R8_UINT);
			switchCase(component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
			switchCase(component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 2, DXGI_FORMAT_R8G8_UNORM);
			switchCase(component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 1, DXGI_FORMAT_R8_UNORM);
			switchCase(component_type::TYPELSEE, 32, 1, DXGI_FORMAT_R32_TYPELESS);
			switchCase(component_type::TYPELSEE, 32, 2, DXGI_FORMAT_R32G32_TYPELESS);
			switchCase(component_type::TYPELSEE, 32, 3, DXGI_FORMAT_R32G32B32_TYPELESS);
			switchCase(component_type::TYPELSEE, 32, 4, DXGI_FORMAT_R32G32B32A32_TYPELESS);
			switchCase(component_type::TYPELSEE, 16, 1, DXGI_FORMAT_R16_TYPELESS);
			switchCase(component_type::TYPELSEE, 16, 2, DXGI_FORMAT_R16G16_TYPELESS);
			switchCase(component_type::TYPELSEE, 16, 4, DXGI_FORMAT_R16G16B16A16_TYPELESS);
			switchCase(component_type::TYPELSEE, 8, 1, DXGI_FORMAT_R8_TYPELESS);
			switchCase(component_type::TYPELSEE, 8, 2, DXGI_FORMAT_R8G8_TYPELESS);
			switchCase(component_type::TYPELSEE, 8, 4, DXGI_FORMAT_R8G8B8A8_TYPELESS);

#undef switchCase
		}

		throw std::nullopt;
	}

	inline constexpr std::optional<DXGI_FORMAT> get_depth_stencil_dxgi_format(component_type componentType, std::uint8_t componentSize, std::uint8_t componentNum) {

		switch (get_format_hash(componentType, componentSize, componentNum))
		{

		case get_format_hash(component_type::UNSIGNED_NORMALIZE_FLOAT, 16, 1):
			return DXGI_FORMAT_D16_UNORM;

		case get_format_hash(component_type::FLOAT, 32, 1):
			return DXGI_FORMAT_D32_FLOAT;
		}
		return std::nullopt;
	}
}