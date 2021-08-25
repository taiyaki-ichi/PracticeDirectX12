#pragma once
#include"resource.hpp"

namespace DX12
{
	class buffer_resource : public resource<resource_dimention::Buffer, unknow_format, resource_heap_property::Upload>
	{
	public:
		virtual ~buffer_resource() = default;

		void initialize(Device* device, std::uint32_t size);
	};

	//
	//
	//

	void DX12::buffer_resource::initialize(Device* device, std::uint32_t size)
	{
		resource::initialize(device, size, 1, 1, 1, nullptr);
	}
}