#pragma once
#include"resource_base.hpp"
#include"../descriptor_heap_type.hpp"

namespace DX12
{
	//深度ステンシル用
	class depth_stencil_buffer : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height);
	};


	//ディスクリプタ用
	template<>
	struct DefaultViewTypeTraits<depth_stencil_buffer> {
		using view_type = view_type::depth_stencil_buffer;
	};
	DefineGetResourcePtr(depth_stencil_buffer)

}