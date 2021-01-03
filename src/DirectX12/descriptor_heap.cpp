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
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
		descHeapDesc.NodeMask = 0;//�}�X�N��0
		descHeapDesc.NumDescriptors = size;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�V�F�[�_���\�[�X�r���[(����ђ萔�AUAV��)

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
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
		descHeapDesc.NodeMask = 0;//�}�X�N��0
		descHeapDesc.NumDescriptors = num * stride;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�V�F�[�_���\�[�X�r���[(����ђ萔�AUAV��)

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

		//�萔�o�b�t�@
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resourcePtr->get()->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->get()->GetDesc().Width);

		//�萔�o�b�t�@�r���[�̍쐬
		device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

		//�߂�l�p
		auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

		//�X�V
		m_offset++;

		return h;

	}

	D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap::create_view(device* device, texture_shader_resource* resourcePtr)
	{
		auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

		//�e�N�X�`��
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resourcePtr->get()->GetDesc().Format;//RGBA(0.0f�`1.0f�ɐ��K��)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

		device->get()->CreateShaderResourceView(resourcePtr->get(), //�r���[�Ɗ֘A�t����o�b�t�@
			&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			heapHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		//�߂�l�p
		auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

		//�X�V
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