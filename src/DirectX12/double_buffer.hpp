#pragma once
#include"resource.hpp"
#include<memory>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;
	class depth_buffer;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
	}

	//�_�u���o�b�t�@�p
	//���\�[�X�o���A�Ƃ�������
	class double_buffer
	{
		IDXGIFactory5* m_factory = nullptr;
		IDXGISwapChain4* m_swap_chain = nullptr;
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_descriptor_heap{};

		std::array<resource, 2> m_buffer{};
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
		void begin_drawing_to_backbuffer(command_list*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

		//�o�b�N�o�b�t�@�ւ̕`�ʂ��I������
		//�`�ʂ���������܂Ńo���A����
		void end_drawing_to_backbuffer(command_list*);

		//�o�b�N�o�b�t�@�̃N���A
		void clear_back_buffer(command_list*);

		//�o�b�t�@���t���b�v
		void flip();
	};
}