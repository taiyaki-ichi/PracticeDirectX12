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
		IDXGIFactory6* m_factory = nullptr;
		IDXGISwapChain4* m_swap_chain = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		std::array<ID3D12Resource*, 2> m_buffer = { nullptr,nullptr };

	public:
		double_buffer() = default;
		~double_buffer();

		//device���Ŏg�p
		//hwnd�͉�
		//queue��device�Ɏ������邩�H�H
		bool initialize(device*,HWND,command_list*);

		//�`�ʊJ�n���̃��\�[�X�o���A
		//�o�b�N�o�b�t�@���`�ʃ^�[�Q�b�g�ɂȂ�܂ő҂�
		void begin_resource_barrior(command_list*);
		//�`�ʏI�����̃��\�[�X�o���A
		//�o�b�N�o�b�t�@�ւ̕`�ʊ�����҂�
		void end_resource_barrior(command_list*);

		//�A���[�^�[�Q�b�g�̎擾
		D3D12_CPU_DESCRIPTOR_HANDLE get_render_target(device*);

		//�o�b�t�@���t���b�v
		void flip();
	};
}