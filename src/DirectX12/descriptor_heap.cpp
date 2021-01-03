#include"descriptor_heap.hpp"
#include"constant_buffer_resource.hpp"
#include"texture_shader_resource.hpp"

namespace ichi
{
	descriptor_heap::~descriptor_heap() {
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
	}

	
	bool descriptor_heap::initialize(device* device, unsigned int size)
	{

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
		descHeapDesc.NodeMask = 0;//マスクは0
		descHeapDesc.NumDescriptors = size;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//シェーダリソースビュー(および定数、UAVも)

		if (FAILED(device->get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << __func__ << " is falied\n";
			return false;
		}

		m_size = size;
		m_increment_size = device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_stride = 1;

		return true;
	}
	

	/*
	bool descriptor_heap::initialize(device* device, unsigned int num, unsigned int stride)
	{
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
		descHeapDesc.NodeMask = 0;//マスクは0
		descHeapDesc.NumDescriptors = num * stride;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//シェーダリソースビュー(および定数、UAVも)

		if (FAILED(device->get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << __func__ << " is falied\n";
			return false;
		}

		m_size = num * stride;
		m_increment_size = device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_stride = stride;

		return true;
	}
	*/
	

	D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap::create_view(device* device, constant_buffer_resource* resourcePtr)
	{
		auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

		//定数バッファ
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resourcePtr->get()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->get()->GetDesc().Width);

		//定数バッファビューの作成
		device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

		//戻り値用
		auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

		//更新
		m_offset++;

		return h;

	}

	D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap::create_view(device* device, texture_shader_resource* resourcePtr)
	{
		auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

		//テクスチャ
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resourcePtr->get()->GetDesc().Format;//RGBA(0.0f～1.0fに正規化)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

		device->get()->CreateShaderResourceView(resourcePtr->get(), //ビューと関連付けるバッファ
			&srvDesc, //先ほど設定したテクスチャ設定情報
			heapHandle//ヒープのどこに割り当てるか
		);

		//戻り値用
		auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

		//更新
		m_offset++;

		return h;
	}

	void descriptor_heap::reset() noexcept
	{
		m_offset = 0;
	}

	ID3D12DescriptorHeap*& descriptor_heap::get_ptr() noexcept
	{
		return m_descriptor_heap;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap::get_gpu_handle(unsigned int num)
	{
		auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_stride) * static_cast<UINT64>(num);
		return h;
	}

}