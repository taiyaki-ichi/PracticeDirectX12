#pragma once
#include"../descriptor_heap_type.hpp"
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



	//ディスクリプタ用
	template<>
	struct ViewTypeTraits<texture_resource> {
		using view_type = view_type::float4_shader_resource;
	};
	DefineGetResourcePtr(texture_resource)

}