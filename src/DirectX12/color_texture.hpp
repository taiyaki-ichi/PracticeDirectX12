#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	ID3D12Resource* create_white_texture(device*);
	ID3D12Resource* create_black_texture(device*);
}