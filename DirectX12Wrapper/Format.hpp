#pragma once
#include<cassert>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class Type {
		Float,//32bit
		Uint,//32bit
		UnsignedNormalizedInt8,//8bit,doublebuffer‚Í‚¾‚¢‚½‚¢‚±‚ê
	};

	class FFormat {
	public:
		DXGI_FORMAT value;
		FFormat(Type t, std::uint16_t num);
	};


	//
	//
	//

	inline FFormat::FFormat(Type t, std::uint16_t num)
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
			switchCase(Type::Float, 1, DXGI_FORMAT_R32_FLOAT);
			switchCase(Type::Float, 2, DXGI_FORMAT_R32G32_FLOAT);
			switchCase(Type::Float, 3, DXGI_FORMAT_R32G32B32_FLOAT);
			switchCase(Type::Float, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
			switchCase(Type::Uint, 1, DXGI_FORMAT_R32_UINT);
			switchCase(Type::Uint, 2, DXGI_FORMAT_R32G32_UINT);
			switchCase(Type::Uint, 3, DXGI_FORMAT_R32G32B32_UINT);
			switchCase(Type::Uint, 4, DXGI_FORMAT_R32G32B32A32_UINT);
			switchCase(Type::UnsignedNormalizedInt8, 1, DXGI_FORMAT_R8_UNORM);
			switchCase(Type::UnsignedNormalizedInt8, 2, DXGI_FORMAT_R8G8_UNORM);
			switchCase(Type::UnsignedNormalizedInt8, 4, DXGI_FORMAT_R8G8B8A8_UNORM);


		default:
			throw"";
		}
		
#undef switchCase
	}

}