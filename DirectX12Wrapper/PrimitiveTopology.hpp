#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class PrimitiveTopology
	{
		TrinagleList= D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	};
}