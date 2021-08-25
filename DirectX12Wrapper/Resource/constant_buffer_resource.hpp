#pragma once
#include"buffer_resource.hpp"

namespace DX12
{
	class constent_buffer_resource : public buffer_resource
	{
	public:
		void initialize(Device* device, std::uint32_t size);
	};

	//
	//
	//

	void DX12::constent_buffer_resource::initialize(Device* device, std::uint32_t size)
	{
		//ƒAƒ‰ƒCƒƒ“ƒg
		size = (size + 0xff) & ~0xff;
		buffer_resource::initialize(device, size);
	}
}