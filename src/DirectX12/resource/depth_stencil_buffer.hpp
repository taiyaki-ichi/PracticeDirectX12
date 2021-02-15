#pragma once
#include"resource_base.hpp"

namespace DX12
{
	//深度ステンシル用
	class depth_stencil_buffer : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height);
	};

}