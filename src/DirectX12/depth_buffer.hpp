#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;

	class depth_buffer
	{
		ID3D12Resource* m_resource = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;
		D3D12_DEPTH_STENCIL_VIEW_DESC m_buffer_view{};

	public:
		depth_buffer() = default;
		~depth_buffer();

		bool initialize(device*, unsigned int windowWidth, unsigned int windowHeight);

		//�`�ʂ���O�ɌĂяo��
		void clear(command_list*);

		//�����_�[�^�[�Q�b�g���w�肷��Ƃ��Ɏg�p
		D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_descriptor_handle();

		ID3D12Resource* get();
	};
}