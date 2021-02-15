#pragma once
#include"../descriptor_heap_type.hpp"
#include"resource_base.hpp"
#include<DirectXTex.h>

namespace DX12
{
	//�e�N�X�`���̃��\�[�X
	class texture_resource : public resource_base
	{
	public:
		bool initialize(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage);
	};



	//�f�B�X�N���v�^�p
	template<>
	struct ViewTypeTraits<texture_resource> {
		using view_type = view_type::float4_shader_resource;
	};
	DefineGetResourcePtr(texture_resource)

}