#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/color_texture.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"scene_data.hpp"
#include"DirectX12/resource.hpp"
#include"DirectX12/color_texture.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include<vector>
#include<string>

namespace DX12
{
	class device;
	class vertex_buffer;
	class index_buffer;
	class command_list;
	class resource;

	class mmd_model
	{

		//���_
		vertex_buffer m_vertex_buffer{};
		//�C���f�b�N�X
		index_buffer m_index_buffer{};

		//�e�N�X�`��
		//�}�e���A���ɂ���ēY�����Ŏw�肳���
		std::vector<resource> m_texture{};

		//�V�[���̃f�[�^�̒萔�o�b�t�@
		resource m_scene_constant_resource{};

		//���̂Ƃ���float4,float3,float,float3��material
		//�}�e���A���̕`�ʖ��ɒ萔�o�b�t�@�����Z�b�g���r���[���쐻
		std::vector<resource> m_material_constant_resource{};

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
		descriptor_heap<descriptor_heap_type::CBV_SRV_UAV> m_descriptor_heap{};

		//�}�e���A�����Ƃ�gpu�n���h���̐擪
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_matarial_root_gpu_handle{};

		//�Ƃ肠�������ƍ��̃e�N�X�`���������o�Ƃ��Ď�������
		white_texture_resource m_white_texture_resource{};
		black_texture_resource m_black_texture_resource{};
		//�O���f�[�V������
		gray_gradation_texture_resource m_gray_gradation_texture_resource{};

		//lightDepth�̃r���[�̃n���h���̃���
		D3D12_GPU_DESCRIPTOR_HANDLE m_light_depth_gpu_handle{};

	public:
		//�R�}���h���X�g�̓e�N�X�`���̃R�s�[�p
		bool initialize(device*, const MMDL::pmx_model<std::wstring>&, command_list*, resource* lightDepthResource);

		void draw(command_list* cl);

		void map_scene_data(const scene_data&);

		//���C�g�[�x�o�b�t�@�ւ̕`��
		void draw_light_depth(command_list* cl);

	};

}