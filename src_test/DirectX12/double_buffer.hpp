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

		//�o�b�t�@���C���N�������g����悤�Ƀ������Ă���
		//begin_drawing_to_backbaffer��device���Q�Ƃ������Ȃ�
		UINT m_descriptor_handle_increment_size = 0;

	public:
		double_buffer() = default;
		~double_buffer();

		//device���Ŏg�p
		//hwnd�͉�
		//queue��device�Ɏ������邩�H�H
		bool initialize(device*,HWND,command_list*);

		//�o�b�N�o�b�t�@�ւ̕`�ʂ��J�n����
		//�o�b�N�o�b�t�@�����\�[�X�o���A���A�����_�[�^�[�Q�b�g�Ɏw��
		//���ƁA�o�b�t�@�̃N���A
		void begin_drawing_to_backbuffer(command_list*);
		//�o�b�N�o�b�t�@�ւ̕`�ʂ��I������
		//�`�ʂ���������܂Ńo���A����
		void end_drawing_to_backbuffer(command_list*);

		//�o�b�t�@���t���b�v
		void flip();
	};
}