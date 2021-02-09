#pragma once
#include"DirectX12/resource_type_tag.hpp"
#include<memory>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
	}

	//�؂�|���S���̃����_���[�֌W
	class perapolygon_renderer
	{
		//�������ݗp�̃��\�[�X�̃r���[�����
		//ID3D12DescriptorHeap* m_descriptor_heap = nullptr;
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_descriptor_heap{};

		//���ۂ̃��\�[�X
		ID3D12Resource* m_resource = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//�؂�|���S���̃��\�[�X�ւ̕`�ʂ̊J�n�ƏI���
		void begin_drawing(command_list*, const D3D12_CPU_DESCRIPTOR_HANDLE&);
		void end_drawing(command_list* cl);

		//���\�[�X�̃N���A
		//�Ǝv�������ǁAinit�ŃN���A�J���[�̂������炢��Ȃ�����
		//�Ƃ肠�����i�V
		//�Ǝv�������ǕK�v���������ۂ�
		void clear(command_list* cl);

		//���\�[�X�̎擾
		//�؂�|���S���ɕ`�ʂ��ꂽ�f�[�^�����H����Ƃ��Ƃ��Ɏg��
		ID3D12Resource* ger_resource_ptr() noexcept;
	};

}