#include"index_buffer.hpp"
#include"device.hpp"

#include<iostream>

namespace ichi
{
	index_buffer::~index_buffer()
	{
		if (m_resource)
			m_resource->Release();
	}
	bool index_buffer::initialize(device* device, unsigned int size)
	{

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resdesc.Width = size;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//Ž¸”s
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)))) {

			std::cout << "vert_index_buufer init is failed\n";
			return false;
		}

		m_buffer_view.BufferLocation = m_resource->GetGPUVirtualAddress();
		m_buffer_view.Format = DXGI_FORMAT_R16_UINT;
		m_buffer_view.SizeInBytes = size;

		return false;
	}
	const D3D12_INDEX_BUFFER_VIEW& index_buffer::get_view() 
	{
		return m_buffer_view;
	}
}