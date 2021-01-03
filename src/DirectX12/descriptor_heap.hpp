#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
#include"resource_type_tag.hpp"
#include<optional>
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

//#include<typeinfo>

namespace ichi
{
	class device;

	//ConstantBuffer�p��ShaderResouce�p
	//sapmer��static��RootSignature�Őݒ�
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_stride = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);

		//�r���[�̍쐻
		//�f�B�X�N���v�^�q�[�v���Z�b�g����悤��gpu_handle���擾�ł���悤�ɂ��Ă���
		//���\�[�X�^�C�v�������Ă���ꍇ�ɌĂ΂��֐�
		template<typename T,typename = typename T::resource_type>
		std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> create_view(device* device, T* resourcePtr);
		//�����Ă��Ȃ��ꍇ
		std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> create_view(...);

		//offset��0�ɂ���
		void reset() noexcept;

		ID3D12DescriptorHeap*& get_ptr() noexcept;

		//�擪����stride�~num���i�񂾃n���h���̈ʒu
		//0�X�^�[�g
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0);
	};





	template<typename T,typename ResourceType>
	inline std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> descriptor_heap::create_view(device* device, T* resourcePtr)
	{
		//std::cout << typeid(ResourceType).name() << "\n";

		//�萔�o�b�t�@�̏ꍇ
		if constexpr (std::is_same_v<ResourceType, constant_buffer_tag>)
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
		else if constexpr (std::is_same_v<ResourceType, shader_resource_tag>)
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

		std::cout << "create view is failed\n";

		return std::nullopt;
	}

}