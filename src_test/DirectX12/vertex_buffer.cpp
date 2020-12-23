#include"vertex_buffer.hpp"
#include"device.hpp"

#include<iostream>

namespace ichi
{
	vertex_buffer::~vertex_buffer()
	{
		if (m_resource)
			m_resource->Release();
	}
	bool vertex_buffer::initialize(device* device, unsigned int size, unsigned int stride)
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

		//失敗
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

		m_buffer_view.BufferLocation = m_resource->GetGPUVirtualAddress();//バッファの仮想アドレス
		m_buffer_view.SizeInBytes = size;//全バイト数
		m_buffer_view.StrideInBytes = stride;//1頂点あたりのバイト数

		return true;
	}
	const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer::get_view() const noexcept
	{
		return m_buffer_view;
	}
}