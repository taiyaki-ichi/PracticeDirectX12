#pragma once
#include<utility>
#include<memory>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
		struct CBV_SRV_UAV;
	}
	class vertex_buffer;
	class command_list;


	class perapolygon
	{
		ID3D12RootSignature* m_root_signature = nullptr;
		ID3D12PipelineState* m_pipeline_state = nullptr;
		ID3D12PipelineState* m_blur_pipeline_state = nullptr;

		//���ۂ̃��\�[�X�A�F�p
		ID3D12Resource* m_color_resource = nullptr;
		//�@���p
		ID3D12Resource* m_normal_resource = nullptr;
		//�u���[���p
		ID3D12Resource* m_bloom_resource = nullptr;
		//�u���[���̍ێg�p����k�����ꂽ�o�b�t�@
		ID3D12Resource* m_shrink_bloom_resource = nullptr;


		//�؂�|���S���̃��\�[�X�Ƀf�[�^���������ޗp
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_rtv_descriptor_heap{};

		//�؂�|���S���̃��\�[�X���e�N�X�`���Ƃ��ĉ��߂����邽�߂�
		std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>> m_cbv_srv_usv_descriptor_heap{};

		//�؂�|���S�����o�b�N�o�b�t�@�ɕ`�ʂ���ۃZ�b�g����p
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};

		//�����_�[�^�[�Q�b�g�Ɏw�肷�鎞�Ɏg��
		D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_cpu_handle_array[3]{};

	public:
		perapolygon() = default;
		~perapolygon();

		bool initialize(device*);

		//�����_�[�^�[�Q�b�g�̐��Ƃ��̐擪�̃n���h���̎擾
		//�؂�|���S���������_�[�^�[�Q�b�g�Ƃ���ۂɎg�p���邱�Ƃ�z��
		std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> get_render_target_info();

		//�؂�|���S���֏������ލۂ̃��\�[�X�o���A
		void begin_drawing_resource_barrier(command_list*);
		//�؂�|���S���ւ̏������݂��I��������ɌĂяo�����\�[�X�o���A
		void end_drawing_resource_barrier(command_list*);

		//���\�[�X�����̏�����
		void clear(command_list*);

		//�؂�|���S������������
		void draw(command_list*);

		//�ڂ������s�����ߏk���o�b�t�@�ւ̏�������
		void draw_shrink_texture_for_blur(command_list*);
	};

}