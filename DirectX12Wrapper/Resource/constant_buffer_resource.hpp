#pragma once
#include"buffer_resource.hpp"
#include"../Utility.hpp"

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
		buffer_resource::initialize(device, Alignment<std::uint32_t>(size, 256));
	}
}