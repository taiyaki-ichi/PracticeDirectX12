#pragma once
#include"upload_resource.hpp"

namespace DX12
{
	//�萔�o�b�t�@
	class constant_buffer : public upload_resource
	{
	public:
		bool initialize(device*, unsigned int size);
	};

}