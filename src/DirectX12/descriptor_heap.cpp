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
	
	
	std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> descriptor_heap::create_view(...)
	{
		std::cout << "create view is failed : resource type is not defined\n";

		return std::nullopt;
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