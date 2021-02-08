#pragma once
#include"DirectX12/resource_type_tag.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;
	class depth_buffer;

	//�؂�|���S���̃����_���[�֌W
	class perapolygon_renderer
	{
		//�������ݗp�̃��\�[�X�̃r���[�����
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		//���ۂ̃��\�[�X
		ID3D12Resource* m_resource = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//���\�[�X�ւ̕`�ʂ̊J�n�ƏI���
		void begin_drawing(command_list* cl, depth_buffer* db);
		void end_drawing(command_list* cl);

		//���\�[�X�̃N���A
		//�Ǝv�������ǁAinit�ŃN���A�J���[�̂������炢��Ȃ�����
		//�Ƃ肠�����i�V
		//�Ǝv�������ǕK�v���������ۂ�
		void clear(command_list* cl);

		//���\�[�X�ɕ`�ʂ����̂��f�B�X�N���v�^�[�q�[�v�Ƀr���[�����p
		//�Ƃ��āA�z��
		//�����������悳��������
		using create_view_type = typename create_view_type::SRV;
		ID3D12Resource* get() noexcept;
	};

}