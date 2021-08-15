#pragma once
#include<cassert>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class Type {
		Float16,
		Float32,
		Uint16,
		Uint32,
		UnsignedNormalizedFloat8,//framebuffer‚Í‚¾‚¢‚½‚¢‚±‚ê
	};

	class Format {
	public:
		DXGI_FORMAT value;
		Format(Type t, std::uint16_t num);
	};


	//
	//
	//

	inline Format::Format(Type t, std::uint16_t num)
	{
		
		constexpr auto getHash = [](Type t, std::uint16_t num) ->std::uint16_t {
			auto typeHash = 1 << (static_cast<std::uint16_t>(t) + 2);
			return typeHash + (num - 1);
		};
		

#define switchCase(t,n,v)		\
case getHash(t,n):				\
	value=v;					\
	break;						\

		switch (getHash(t, num))
		{
			switchCase(Type::Float16, 1, DXGI_FORMAT_R16_FLOAT);
			switchCase(Type::Float16, 2, DXGI_FORMAT_R16G16_FLOAT);
			switchCase(Type::Float16, 4, DXGI_FORMAT_R16G16B16A16_FLOAT);
			switchCase(Type::Float32, 1, DXGI_FORMAT_R32_FLOAT);
			switchCase(Type::Float32, 2, DXGI_FORMAT_R32G32_FLOAT);
			switchCase(Type::Float32, 3, DXGI_FORMAT_R32G32B32_FLOAT);
			switchCase(Type::Float32, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
			switchCase(Type::Uint16, 1, DXGI_FORMAT_R16_UINT);
			switchCase(Type::Uint16, 2, DXGI_FORMAT_R16G16_UINT);
			switchCase(Type::Uint16, 4, DXGI_FORMAT_R16G16B16A16_UINT);
			switchCase(Type::Uint32, 1, DXGI_FORMAT_R32_UINT);
			switchCase(Type::Uint32, 2, DXGI_FORMAT_R32G32_UINT);
			switchCase(Type::Uint32, 3, DXGI_FORMAT_R32G32B32_UINT);
			switchCase(Type::Uint32, 4, DXGI_FORMAT_R32G32B32A32_UINT);
			switchCase(Type::UnsignedNormalizedFloat8, 1, DXGI_FORMAT_R8_UNORM);
			switchCase(Type::UnsignedNormalizedFloat8, 2, DXGI_FORMAT_R8G8_UNORM);
			switchCase(Type::UnsignedNormalizedFloat8, 4, DXGI_FORMAT_R8G8B8A8_UNORM);

		default:
			throw"";
		}
		
#undef switchCase
	}

}