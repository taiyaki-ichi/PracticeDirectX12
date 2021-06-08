#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	enum class ResourceState {
		Common = D3D12_RESOURCE_STATE_COMMON,
		RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
		PixcelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	};
}