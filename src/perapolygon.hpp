#pragma once
#include"DirectX12/descriptor_heap.hpp"
#include"scene_data.hpp"
#include"DirectX12/resource/vertex_buffer.hpp"
#include"DirectX12/resource/constant_buffer.hpp"
#include"DirectX12/resource/float_shader_resource.hpp"
#include"DirectX12/resource/float4_shader_resource.hpp"
#include<utility>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
		struct CBV_SRV_UAV;
	}
	class command_list;

	class perapolygon
	{
		//���\�[�X����

		constexpr static unsigned int RESOURCE_NUM = 7;

		std::array<float4_shader_resource, 5> m_float4_resource{};
		enum Float4ResourceIndex {
			COLOR,//�F
			NORMAL,//�@��
			BLOOM,//�u���[���p
			SHRINK_BLOOM,//�u���[���p�̏k�����ꂽ���\�[�X
			DOF,//��ʊE�[�x�ڂ����t�B���^�p
		};
		//�X�N���[���X�y�[�X�A���r�G���g�I�N���[�W�����p
		float_shader_resource m_SSAO_resource{};
		//�V�[���̃f�[�^�Ȃǂ��i�[
		constant_buffer m_constant_buffer{};


		//�؂�|���S���̃��\�[�X�Ƀf�[�^���������ޗp
		descriptor_heap<descriptor_heap_type::RTV> m_rtv_descriptor_heap{};

		//�؂�|���S���̃��\�[�X���e�N�X�`���Ƃ��ĉ��߂����邽�߂�
		descriptor_heap<descriptor_heap_type::CBV_SRV_UAV> m_cbv_srv_usv_descriptor_heap{};

		//�؂�|���S�����o�b�N�o�b�t�@�ɕ`�ʂ���ۃZ�b�g����p
		vertex_buffer m_vertex_buffer{};

		//�����_�[�^�[�Q�b�g�Ɏw�肷�鎞�Ɏg��
		static constexpr unsigned int RENDER_TARGET_HANDLE_NUM = 3;
		D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_cpu_handle_array[RENDER_TARGET_HANDLE_NUM]{};

	public:

		//�[�x�o�b�t�@��View����肽���̂ň����ɐ[�x�o�b�t�@
		bool initialize(device*, ID3D12Resource* depthResource);

		//�����_�[�^�[�Q�b�g�̐��Ƃ��̐擪�̃n���h���̎擾
		//�؂�|���S���������_�[�^�[�Q�b�g�Ƃ���ۂɎg�p���邱�Ƃ�z��
		std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> get_render_target_info();

		//���ׂẴ��\�[�X�̃��\�[�X�o���A
		void all_resource_barrior(command_list*, D3D12_RESOURCE_STATES);

		//SSAO�̃o���A
		//������
		void ssao_resource_barrior(command_list*, D3D12_RESOURCE_STATES);

		//������������
		D3D12_CPU_DESCRIPTOR_HANDLE get_ssao_cpu_handle();

		//CBV�ɃV�[���̃f�[�^��n��
		void map_scene_data(const scene_data_for_perapolygon&);

		//���\�[�X�����̏�����
		void clear(command_list*);

		//�؂�|���S������������
		void draw(command_list*);

		//�ڂ������s�����ߏk���o�b�t�@�ւ̏�������
		void draw_shrink_texture_for_blur(command_list*);
	};

}