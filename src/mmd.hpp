#pragma once
#include"DirectX12/texture_shader_resource.hpp"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace ichi
{

	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	class device;
	class command_list;
	class descriptor_heap;
	class pipeline_state;


	//pmx_vertex����K�v�ȏ��𒊏o
	//�����A�X�V���Ă���
	struct my_vertex
	{
		//���_���W
		DirectX::XMFLOAT3 m_position{};
		//�@��
		DirectX::XMFLOAT3 m_normal{};
		//UV���W
		DirectX::XMFLOAT2 m_uv{};
	};

	struct my_material {
		//MMDL��material��3���Ⴀ�Ȃ���4������
		//�Ƃ肠������FLOAT3�ł��
		DirectX::XMFLOAT4 m_diffuse;
		DirectX::XMFLOAT3 m_specular;
		float m_specularity;
		DirectX::XMFLOAT3 m_ambient;
	};

	struct my_material_info {
		//m_texture�̃T�C�Y��葽���Ȓl�͖���
		unsigned int m_texture_index = 0;
		unsigned int m_toon_index = 0;
		//���_���w���C���f�b�N�X�̐�
		int m_vertex_num = 0;
	};

	//mmd�̃N���X
	class my_mmd
	{
		//���_
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};
		//�C���f�b�N�X
		std::unique_ptr<index_buffer> m_index_buffer{};

		//�e�N�X�`��
		//�}�e���A���ɂ���ēY�����Ŏw�肳���
		std::vector<std::unique_ptr<texture_shader_resource>> m_texture{};

		//�V�F�[�_�ɑ���p
		//�Ƃ肠�����A�����o�Ƃ��ĕێ����Ă���
		std::unique_ptr<constant_buffer_resource> m_world_mat_resource{};
		std::unique_ptr<constant_buffer_resource> m_viewproj_mat_resource{};

		//���̂Ƃ���float4,float3,float,float3��my_material
		std::vector<std::unique_ptr<constant_buffer_resource>> m_material_resource{};
		
		//m_material_resource�ɑΉ������Y����
		std::vector<my_material_info> m_material_info{};
		
	public:
		my_mmd() = default;
		~my_mmd() = default;

		//�Ƃ肠����wstring����
		bool initialize(device*, std::vector<my_vertex>&, std::vector<unsigned short>& index,
			std::vector<my_material>&, std::vector<my_material_info>&, std::vector<std::wstring>& filePath,
			command_list*);

		//DoubleBuffer��begin�A�v���~�e�B�u�A�V�U�[��`�A�r���[�|�[�g��ݒ肵����Ăяo��
		//i�Ԗڂ̃}�e���A���̕`��
		void draw_command(command_list*, descriptor_heap*, device*, unsigned int i);

		//�A�e���A���̐���Ԃ�
		unsigned int get_material_num();

		void map_world_mat(DirectX::XMMATRIX&);
		void map_viewproj_mat(DirectX::XMMATRIX&);


	};

}