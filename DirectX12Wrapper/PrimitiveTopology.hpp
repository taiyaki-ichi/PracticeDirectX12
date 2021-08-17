#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class PrimitiveTopology
	{
		Triangle = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		Contorol4PointPatchList = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
		PointList = D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		Patch = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
	};
}