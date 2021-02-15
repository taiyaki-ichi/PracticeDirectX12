#pragma once
#include"upload_resource.hpp"

namespace DX12
{
	//定数バッファ
	class constant_buffer : public upload_resource
	{
	public:
		bool initialize(device*, unsigned int size);
	};

}