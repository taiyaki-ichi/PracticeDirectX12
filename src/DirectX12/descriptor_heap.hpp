#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
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
		//bool initialize(device* d, unsigned int num, unsigned int stride);

		//�r���[�̍쐻
		//�f�B�X�N���v�^�q�[�v���Z�b�g����悤��gpu_handle���擾�ł���悤�ɂ��Ă���
		D3D12_GPU_DESCRIPTOR_HANDLE create_view(device* device, constant_buffer_resource* resourcePtr);
		D3D12_GPU_DESCRIPTOR_HANDLE create_view(device* device, texture_shader_resource* resourcePtr);

		//offset��0�ɂ���
		void reset() noexcept;

		ID3D12DescriptorHeap*& get_ptr() noexcept;

		//�擪����stride�~num���i�񂾃n���h���̈ʒu
		//0�X�^�[�g
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0);
	};

}