#pragma once
#include"device.hpp"
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//��
	constexpr unsigned int DESCRIPTOR_HEAP_SIZE = 128;

	//ConstantBuffer�p��ShaderResouce�p��2���
	//sapmer��static��RootSignature�Őݒ�
	template<typename Value>
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);

		//�`�ʖ��ɌĂяo���ăr���[���쐻
		//�e�N�X�`�����萔�o�b�t�@���ŏ������ς��
		//����󂯓����悤�ɂ��邩
		bool create_view(device* device, Value* resourcePtr);

		//offset��0�ɂ���
		void reset() noexcept;

		ID3D12DescriptorHeap*& get() noexcept;
	};



	template<typename Value>
	inline descriptor_heap<Value>::~descriptor_heap(){
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
	}

	template<typename Value>
	inline bool descriptor_heap<Value>::initialize(device* device, unsigned int size)
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

		return true;
	}

	template<typename Value>
	inline bool descriptor_heap<Value>::create_view(device* device, Value* resourcePtr)
	{

		auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		heapHandle.ptr += m_increment_size * m_offset;

		if constexpr (std::is_same_v<Value, constant_buffer_resource>) {
			//�萔�o�b�t�@
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resourcePtr->get()->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->get()->GetDesc().Width);

			//�萔�o�b�t�@�r���[�̍쐬
			device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

			m_offset++;

			return true;
		}
		else if constexpr (std::is_same_v<Value, texture_shader_resource>) {
			//�e�N�X�`��
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = resourcePtr->get()->GetDesc().Format;//RGBA(0.0f�`1.0f�ɐ��K��)
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
			srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

			device->get()->CreateShaderResourceView(resourcePtr, //�r���[�Ɗ֘A�t����o�b�t�@
				&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
				heapHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
			);

			m_offset++;

			return true;
		}

		return false;
	}

	template<typename Value>
	inline void descriptor_heap<Value>::reset() noexcept
	{
		m_offset = 0;
	}

	template<typename Value>
	inline ID3D12DescriptorHeap*& descriptor_heap<Value>::get() noexcept
	{
		return m_descriptor_heap;
	}



}