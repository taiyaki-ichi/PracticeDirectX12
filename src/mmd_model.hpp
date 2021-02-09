#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"DirectX12/color_texture.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"scene_data.hpp"
#include<memory>
#include<vector>
#include<string>

namespace ichi
{
	class device;
	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	class command_list;
	class white_texture_resource;
	class black_texture_resource;

	class mmd_model
	{
		//�ʏ�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//�V���h�E�p�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* m_shadow_pipeline_state = nullptr;
		//���[�g�V�O�l�`��
		ID3D12RootSignature* m_root_signature = nullptr;

		//���_
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};
		//�C���f�b�N�X
		std::unique_ptr<index_buffer> m_index_buffer{};

		//�e�N�X�`��
		//�}�e���A���ɂ���ēY�����Ŏw�肳���
		std::vector<std::unique_ptr<texture_shader_resource>> m_texture{};

		//�V�F�[�_�ɑ���p
		std::unique_ptr<constant_buffer_resource> m_scene_data_resource{};

		//���̂Ƃ���float4,float3,float,float3��material
		//�}�e���A���̕`�ʖ��ɒ萔�o�b�t�@�����Z�b�g���r���[���쐻
		std::vector<std::unique_ptr<constant_buffer_resource>> m_material_resource{};

		//�}�e���A���̏��ێ��p
		struct material_info {
			//m_texture�̃T�C�Y��葽���Ȓl�͖���
			//����2�͂���Ȃ�����
			unsigned int m_texture_index = 0;
			unsigned int m_toon_index = 0;
			//���_���w���C���f�b�N�X�̐�
			int m_vertex_num = 0;
		};
		//�}�e���A���̏��̕ێ�
		std::vector<material_info> m_material_info{};

		//mmd�̂��ׂẴC���f�b�N�X�̐�
		size_t m_all_index_num{};

		//�f�B�X�N���v�^�q�[�v
		//SetDescriptorHeaps�𕡐���Ăяo���Ƃ��܂������Ȃ������̂ł�����߂�1�ɂ܂Ƃ߂�
		std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>> m_descriptor_heap{};

		//�}�e���A�����Ƃ�gpu�n���h���̐擪
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_matarial_root_gpu_handle{};

		//�Ƃ肠�������ƍ��̃e�N�X�`���������o�Ƃ��Ď�������
		std::unique_ptr<white_texture_resource> m_white_texture_resource{};
		std::unique_ptr<black_texture_resource> m_black_texture_resource{};
		//�O���f�[�V������
		std::unique_ptr<gray_gradation_texture_resource> m_gray_gradation_texture_resource{};

		//�[�x�������ݗp�̃o�b�t�@
		ID3D12Resource* m_depth_resource = nullptr;
		//�V���h�E�}�b�v�p�̐[�x�o�b�t�@
		ID3D12Resource* m_light_depth_resource = nullptr;
		//�[�x�o�b�t�@�ƃ��C�g�[�x�o�b�t�@�p�̃f�B�X�N���v�^�q�[�v
		//���ʂ̂�0�ԖځA���C�g�p��1�ԖڂƂȂ�悤��View���쐻��
		std::unique_ptr<descriptor_heap<descriptor_heap_type::DSV>> m_depth_descriptor_heap{};

		//�`�ʗp��GPU�n���h��
		D3D12_GPU_DESCRIPTOR_HANDLE m_light_depth_gpu_handle{};
	

	public:
		mmd_model() = default;
		~mmd_model();

		//�R�}���h���X�g�̓e�N�X�`���̃R�s�[�p
		bool initialize(device*,const MMDL::pmx_model<std::wstring>&,command_list*);


		//
		//��
		//draw�̓R�}���h���X�g�̃����o�֐��ɂ�������
		//
		void draw(command_list* cl);

		void map_scene_data(const scene_data&);

		//���C�g�[�x�o�b�t�@�ւ̕`��
		void draw_light_depth(command_list* cl);

		D3D12_CPU_DESCRIPTOR_HANDLE get_depth_resource_cpu_handle() const noexcept;

	};

}