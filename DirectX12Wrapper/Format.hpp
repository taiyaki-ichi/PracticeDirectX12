#pragma once
#include<cassert>
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class component_type
	{
		FLOAT,
		UINT,
		UNSIGNED_NORMALIZE_FLOAT,
		TYPELSEE,
	};

	struct format
	{
		component_type type;
		std::uint32_t size;
		std::uint32_t num;
	};

	inline constexpr std::optional<DXGI_FORMAT> get_dxgi_format(component_type type, std::uint32_t size, std::uint32_t num)
	{
		//0-3bit‚ÅnumA4-6‚ÅsizeAŽc‚è‚ªtype
		constexpr auto getHash = [](component_type type, std::uint32_t size, std::uint32_t num) constexpr ->std::uint32_t {
			std::uint32_t numHash = 1 << (num - 1);
			std::uint32_t sizeHash = (1 << 4) << ((size / 8) >> 1);
			std::uint32_t typeHash = (1 << 7) << static_cast<std::uint32_t>(type);
			return numHash | sizeHash | typeHash;
		};

		switch (getHash(type, size, num))
		{

#define switchCase(t,s,n,v)								\
			case getHash(t,s,n): return v;				\

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

		return std::nullopt;
	}


}