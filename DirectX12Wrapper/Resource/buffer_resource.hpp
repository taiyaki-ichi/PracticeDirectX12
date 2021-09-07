#pragma once
#include"resource.hpp"

namespace DX12
{
	template<resource_heap_property HeapProperty>
	class buffer_resource : public resource<resource_dimention::Buffer, unknow_format, HeapProperty>
	{
	public:
		buffer_resource() = default;
		virtual ~buffer_resource() = default;

		buffer_resource(buffer_resource&&) = default;
		buffer_resource& operator=(buffer_resource&&) = default;

		void initialize(device& device, std::uint32_t size);
	};

	//
	//
	//

	template<resource_heap_property HeapProperty>
	void DX12::buffer_resource<HeapProperty>::initialize(device& device, std::uint32_t size)
	{
		resource<resource_dimention::Buffer, unknow_format, HeapProperty>::initialize(device, size, 1, 1, 1);
	}
}