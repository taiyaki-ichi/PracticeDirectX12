#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//mmdの描写に使用する深度バッファとライト深度バッファの作製
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device*, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition);

	

}