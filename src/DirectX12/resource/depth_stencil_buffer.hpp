#pragma once
#include"resource_base.hpp"
#include"../descriptor_heap_type.hpp"

namespace DX12
{
	//�[�x�X�e���V���p
	class depth_stencil_buffer : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height);
	};


	//�f�B�X�N���v�^�p
	template<>
	struct DefaultViewTypeTraits<depth_stencil_buffer> {
		using view_type = view_type::depth_stencil_buffer;
	};
	DefineGetResourcePtr(depth_stencil_buffer)

}