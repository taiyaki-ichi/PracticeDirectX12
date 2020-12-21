#pragma once
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;

	//�_�u���o�b�t�@�p
	//���\�[�X�o���A�Ƃ�������
	class double_buffer
	{
		IDXGISwapChain4* m_swap_chain = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		std::array<ID3D12Resource*, 2> m_buffer = { nullptr,nullptr };

	public:
		double_buffer() = default;
		~double_buffer();

		//device���Ŏg�p
		bool initialize(device*);

		//�Ƃ肠�����̓o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�ɂȂ�܂Ńo���A
		//���̂ق��̏�Ԃ܂Ńo���A���邩�͌�����
		void resource_barrior(command_list*);

		//�o�b�t�@���t���b�v
		bool flip();

	};
}