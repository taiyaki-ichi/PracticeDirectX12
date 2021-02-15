#pragma once
#include"resource_base.hpp"
#include<DirectXTex.h>

namespace DX12
{
	//テクスチャのリソース
	class texture_resource : public resource_base
	{
	public:
		bool initialize(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage);
	};
}