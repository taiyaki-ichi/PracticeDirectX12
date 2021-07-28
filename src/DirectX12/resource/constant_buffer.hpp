#pragma once
#include"upload_resource.hpp"
#include"../descriptor_heap_type.hpp"

namespace DX12
{
	//定数バッファ
	class constant_buffer : public upload_resource
	{
	public:
		bool initialize(device*, unsigned int size);
	};


	//ディスクリプタ用
	template<>
	struct DefaultViewTypeTraits<constant_buffer> {
		using view_type = view_type::constant_buffer;
	};
	DefineGetResourcePtr(constant_buffer)
}