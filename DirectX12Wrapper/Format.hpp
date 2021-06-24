#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class Format {
		R8G8B8A8 = DXGI_FORMAT_R8G8B8A8_UNORM,
		R32_FLOAT = DXGI_FORMAT_R32_FLOAT,
	};

}