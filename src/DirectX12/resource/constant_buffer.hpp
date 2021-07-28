#pragma once
#include"upload_resource.hpp"
#include"../descriptor_heap_type.hpp"

namespace DX12
{
	//�萔�o�b�t�@
	class constant_buffer : public upload_resource
	{
	public:
		bool initialize(device*, unsigned int size);
	};


	//�f�B�X�N���v�^�p
	template<>
	struct DefaultViewTypeTraits<constant_buffer> {
		using view_type = view_type::constant_buffer;
	};
	DefineGetResourcePtr(constant_buffer)
}