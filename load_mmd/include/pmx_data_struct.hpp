#pragma once
#include<DirectXMath.h>
#include<vector>
#include<variant>
#include<array>

namespace MMDL
{

	struct pmx_header
	{
		//data�̃C���f�b�N�X�p
		enum data_index
		{
			ENCODING_FORMAT,
			NUMBER_OF_ADD_UV,
			VERTEX_INDEX_SIZE,
			TEXTURE_INDEX_SIZE,
			MATERIAL_INDEX_SIZE,
			BONE_INDEX_SIZE,
			RIGID_BODY_INDEX_SIZE
		};

		//�o�[�W����
		float m_version{};
		//�o�C�g�T�C�Y
		unsigned char m_data_size{};
		//�o�C�g��
		unsigned char m_data[8]{};
	};

	//CharType��char��wchar_t��z��
	//pmd_header.m_byte[0]��0�Ȃ當���R�[�h��UTF16�Ȃ̂�wchar_t
	//1�Ȃ�UTF8�Ȃ̂�char
	template<typename StringType>
	struct pmx_info
	{
		//���ꂼ��ő�l�͂Ƃ肠����
		static constexpr size_t MAX_SIZE = 256;

		enum data_index {
			MODEL_NAME,
			MODEL_NAME_ENG,
			COMMENT,
			COMMENT_ENG
		};

		StringType m_data[4]{};
	};


	//���_���
	struct pmx_vertex
	{

		//���_���W
		DirectX::XMFLOAT3 m_position{};
		//�@��
		DirectX::XMFLOAT3 m_normal{};
		//UV���W
		DirectX::XMFLOAT2 m_uv{};
		//�ǉ�UV
		std::vector<DirectX::XMFLOAT4> m_additional_uv{};

		//�{�[���E�F�C�g�Ƃ�
		//�Ƃ肠�����z��Ŏ����A�s�ւ�������J�G��
		//�����l��pmxEditor���Č��߂��A�s�K�؂Ȃ�J�G��
		
		enum bone_type_flag  : std::uint16_t {
			//�P��{�[���݂̂���e�����󂯂�B�܂�m_born[0]�̂�
			BDEF1 = 0,

			//2�̃{�[������e�����󂯂�Bm_borm[0],m_born[1]�̃C���f�b�N�X���g�p
			//�܂��Am_weight[0]�Ɉ�ڂ̃{�[���̃E�F�C�g
			//1-m_weight[0]��2�ڂ̃{�[���̃E�F�C�g
			BDEF2 = 1,

			//4�̃{�[������e�����󂯂�B
			//m_born[0-3],m_weight[0-3]�̂��ꂼ�ꂪ���ꂼ��̃f�[�^
			//m_wieght�̍��v���P�Ƃ͂Ȃ�Ȃ����Ƃ����邩���Ȃ̂Œ���
			BDEF4 = 2,

			//��R�Ƃ͏����قȂ�`��
			//m_born[0],m_born[1]�Ƀ{�[���̃C���f�b�N�X
			//m_weight[0]��1�ڂ̃{�[���̃E�F�C�g�A2�ڂ�1-m_weight[0]
			//m_SDEF_vector[0]=SDEF-C�l
			//m_SDEF_vector[1]=SDEF-R0�l
			//m_SDEF_vector[2]=SDEF-R1�l
			SDEF = 3,
		};

		bone_type_flag m_bone_type_flag{};
		std::array<unsigned short, 4> m_bone{ 0,0,0,0 };
		std::array<float, 4> m_weight{ 0.f,0.f,0.f,0.f };
		std::array<DirectX::XMFLOAT3, 3> m_SDEF_vector{ {{0.f,0.f,0.f},{0.f,0.f,0.f},{0.f,0.f,0.f} } };

		//�G�b�W�{��
		float m_edge_magnification{};
	};

	//��
	struct pmx_surface {
		int m_vertex_index = 0;
	};

	//�}�e���A��
	template<typename StringType>
	struct pmx_material
	{
		StringType m_name{};
		StringType m_name_eng{};

		DirectX::XMFLOAT4 m_diffuse{};
		DirectX::XMFLOAT3 m_specular{};
		float m_specularity = 0.f;
		DirectX::XMFLOAT3 m_ambient{};

		//�`��t���O
		//����bit����A���ʕ`��A�n�ʉe�A�Z���t�V���h�E�}�b�v�ւ̕`��
		//�Z���t�V���h�E�̕`��A�G�b�W�`��A�̏�
		unsigned char m_drawing_flag = 0;

		//�G�b�W�F(R,G,B,A)
		DirectX::XMFLOAT4 m_edge_color{};
		//�G�b�W�T�C�Y
		float m_edge_size = 0;

		//�ʏ�e�N�X�`���A�e�N�X�`���e�[�u���̎Q��Index
		unsigned int m_texture_index_size_1 = 0;
		//�X�t�B�A�e�N�X�`���A�e�N�X�`���e�[�u���̎Q��Index�i�e�N�X�`���̊g���q�̐����Ȃ��j
		unsigned int m_texture_index_size_2 = 0;
		//�X�t�B�A���[�h
		//�O�F�����@�P�F��Z�@�Q�F���Z�@�R�F�T�u�e�N�X�`��
		unsigned char m_sphere_mode = 0;;

		//unsigned int�Ȃ��toon�Aunsigned char�Ȃ狤�Ltoon
		//�ꉞ�Ƃ肠�����A�^�Ŏ��ʂł���悤�ɂ��Ă���
		std::variant<unsigned int, unsigned char> m_toon{};

		//�����A���R��
		StringType m_memo{};

		//�ގ��ɑΉ�����ʁi���_�j���i�K��3�̔{���j
		int m_vertex_num = -1;

	};

	//�{�[��
	template<typename StringType>
	struct pmx_bone
	{
		StringType m_name{};
		StringType m_name_eng{};

		//�ʒu
		DirectX::XMFLOAT3 m_position{};
		//�e�{�[���̃C���f�b�N�X
		std::int16_t m_parent_index = -1;
		//�ό`�K�w
		int m_transformation_level = -1;

		//�{�[���t���O
		unsigned short m_flag = 0;

		//���W�̂����ӂ����ƁA�{�[���̈ʒu����̑��Ε�
		DirectX::XMFLOAT3 m_offset{};
		//�ڑ���r�[���̃{�[��Index
		int m_children_index = -1;
		//�t�^�e�{�[���̃{�[��Index
		int m_impart_parent_index = -1;
		//�t�^��
		float m_impart_rate = 0.f;
		//�Œ莲�̕����x�N�g��
		DirectX::XMFLOAT3 m_fixed_axis{};
		//���[�J����X���̕����x�N�g��
		DirectX::XMFLOAT3 m_local_axis_x{};
		//���[�J����Z���̕����x�N�g��
		DirectX::XMFLOAT3 m_local_axis_y{};
		//�O���e�ό`
		int m_external_parent_key = -1;

		//IK�^�[�Q�b�g�{�[���̃{�[��Index
		int m_IK_target_index = -1;
		//IK���[�v��
		int m_IK_loop_count = -1;
		//IK�邤�[�Ռv�Z����1�񓖂���̐����p�x
		float m_IK_unit_angle = 0.f;
		struct IK_link {
			int m_index = -1;
			unsigned char m_exist_angle_limited = 0;
			DirectX::XMFLOAT3 m_limit_angle_min{};
			DirectX::XMFLOAT3 m_limit_angle_max{};
		};
		std::vector<IK_link> m_IK_link{};

	};


	//�S���܂Ƃ߂����
	template<typename StringType>
	struct pmx_model
	{
		pmx_header m_header;
		pmx_info<StringType> m_info;
		std::vector<pmx_vertex> m_vertex;
		std::vector<pmx_surface> m_surface;
		std::vector<StringType> m_texture_path;
		std::vector<pmx_material<StringType>> m_material;
		std::vector<pmx_bone<StringType>> m_bone;
	};

}