#pragma once
#include"utility.hpp"
#include<DirectXMath.h>
#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<variant>
#include<string>
#include<codecvt>
#include<locale>
#include<fstream>

namespace ichi
{


#pragma pack(1)
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

		//"Pmd "�X�y�[�X����
		unsigned char m_magic[4];
		//�o�[�W����
		float m_version;
		//�o�C�g�T�C�Y
		unsigned char m_data_size;
		//�o�C�g��
		unsigned char m_data[8];
	};
#pragma pack()

	//CharType��char��wchar_t��z��
	//pmd_header.m_byte[0]��0�Ȃ當���R�[�h��UTF16�Ȃ̂�wchar_t
	//1�Ȃ�UTF8�Ȃ̂�char
	template<typename CharType>
	struct pmx_model_info
	{
		//���ꂼ��ő�l�͂Ƃ肠����
		static constexpr size_t MAX_SIZE = 256;

		enum data_index {
			MODEL_NAME,
			MODEL_NAME_ENG,
			COMMENT,
			COMMENT_ENG
		};

		CharType m_data[4];
	};


	//���_���
	struct pmx_vertex
	{
		//�{�[���̃E�F�C�g�\����s
		struct born_weight_type_BDEF1 {
			int m_born[1];
		};
		struct born_weight_type_BDEF2 {
			int m_born[2];
			float m_weight[1];
		};
		struct born_weight_type_BDEF4 {
			int m_born[4];
			float m_weight[4];
		};
		struct born_weight_type_SDEF {
			int m_born[2];
			float m_weight[1];
			DirectX::XMFLOAT3 m_SDEF_vector[3];
		};


		//���_���W
		DirectX::XMFLOAT3 m_position;
		//�@��
		DirectX::XMFLOAT3 m_normal;
		//UV���W
		DirectX::XMFLOAT2 m_uv;
		//�ǉ�UV
		std::vector<DirectX::XMFLOAT4> m_additional_uv;

		//�{�[���E�F�C�g
		std::variant<born_weight_type_BDEF1, born_weight_type_BDEF2,
			born_weight_type_BDEF4, born_weight_type_SDEF> m_born_weight;

		//�G�b�W�{��
		float m_edge_magnification;
	};
	
	//�ǂݍ��ނƂ��Ɏg�p
	//�ǂݍ��݂����ɂ����g��Ȃ����ۂ�����w�b�_�ɒu���Ȃ��Ă��ǂ�����
	enum pmx_vertex_weight_type
	{
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF
	};

	//�ʁI
	struct pmx_surface {
		int m_vertex_index;
	};

	inline bool load_pmx(const char* fileName) {
	//inline std::vector<DirectX::XMFLOAT3> load_pmx(const char* fileName) {

		//wcout�g���Ƃ��ɕK�v
		std::wcout.imbue(std::locale(""));

		std::ifstream file{ fileName, std::ios::in | std::ios::binary };
		
		if (!file) {
			//std::cout << "fopen is failed\n";
			return false;
		}


		//�w�b�_
		pmx_header header{};
		{
			read_binary_from_file(file, &header);
			
			//std::cout << header.m_magic << "\n";
			//std::cout << header.m_version << "\n";
			//std::cout << header.m_data_size << "\n";
			//for (size_t i = 0; i < 8; i++)
				//std::cout << static_cast<unsigned int>(header.m_data[i]) << "\n";
				
		}
			

		//���f���̏��
		pmx_model_info<std::wstring> modelInfo{};
		{
			unsigned int num;
			wchar_t tmpStr[pmx_model_info<std::wstring>::MAX_SIZE];
			for (unsigned int i = 0; i < 4; i++) {
				read_binary_from_file(file, &num);
				read_binary_from_file(file, tmpStr, num);
				modelInfo.m_data[i] = std::wstring{ tmpStr };
			}

			//for (int i = 0; i < 4; i++)
				//std::wcout << modelInfo.m_data[i] << "\n";
			
		}

		
		//���_�����
		std::vector<pmx_vertex> vertex{};
		{
			int vertexNum;
			read_binary_from_file(file, &vertexNum);

			//std::cout << vertexNum << "\n";

			vertex.reserve(vertexNum);

			pmx_vertex tmpVertex;
			pmx_vertex::born_weight_type_BDEF1 tmpBDEF1{};
			pmx_vertex::born_weight_type_BDEF2 tmpBDEF2{};
			pmx_vertex::born_weight_type_BDEF4 tmpBDEF4{};
			pmx_vertex::born_weight_type_SDEF tmpSDEF{};
			for (int i = 0; i < vertexNum; i++)
			{
				read_binary_from_file(file, &tmpVertex.m_position);
				read_binary_from_file(file, &tmpVertex.m_normal);
				read_binary_from_file(file, &tmpVertex.m_uv);

				if (header.m_data[pmx_header::NUMBER_OF_ADD_UV] != 0) {
					auto num = header.m_data[pmx_header::NUMBER_OF_ADD_UV];
					tmpVertex.m_additional_uv.resize(num);
					size_t size = header.m_data[pmx_header::NUMBER_OF_ADD_UV] * sizeof(tmpVertex.m_additional_uv[0]);
					read_binary_from_file(file, &tmpVertex.m_additional_uv, size);
				}

				unsigned char weightMethod;
				read_binary_from_file(file, &weightMethod);

				switch (weightMethod)
				{
				case pmx_vertex_weight_type::BDEF1:
					read_binary_from_file(file, &tmpBDEF1.m_born, header.m_data[pmx_header::BONE_INDEX_SIZE]);
					tmpVertex.m_born_weight = std::move(tmpBDEF1);
					break;

				case pmx_vertex_weight_type::BDEF2:
					read_binary_from_file(file, &tmpBDEF2.m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 2);
					read_binary_from_file(file, &tmpBDEF2.m_weight, sizeof(&tmpBDEF2.m_weight[0]));
					tmpVertex.m_born_weight = std::move(tmpBDEF2);
					break;

				case pmx_vertex_weight_type::BDEF4:
					read_binary_from_file(file, &tmpBDEF4.m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 4);
					read_binary_from_file(file, &tmpBDEF4.m_weight, sizeof(&tmpBDEF4.m_weight[0]) * 4);
					tmpVertex.m_born_weight = std::move(tmpBDEF4);
					break;

				case pmx_vertex_weight_type::SDEF:
					read_binary_from_file(file, &tmpSDEF.m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 2);
					read_binary_from_file(file, &tmpSDEF.m_weight, sizeof(tmpSDEF.m_weight[0]));
					read_binary_from_file(file, &tmpSDEF.m_SDEF_vector, sizeof(tmpSDEF.m_SDEF_vector[0]) * 3);
					tmpVertex.m_born_weight = std::move(tmpSDEF);
					break;
				}

				read_binary_from_file(file, &tmpVertex.m_edge_magnification);

				vertex.emplace_back(std::move(tmpVertex));
			}

			//for (int i = 0; i < 10; i++) {
				//std::cout << vertex[i].m_position.x << "," << vertex[i].m_position.y << "," << vertex[i].m_position.z << "\n";
			//}
			
			//std::vector<DirectX::XMFLOAT3> vert{};
			//vert.reserve(vertex.size());
			//for (int i = 0; i < vertex.size(); i++)
				//vert.emplace_back(std::move(vertex[i].m_position));

			//return vert;
		}


		//��
		std::vector<pmx_surface> surface{};
		{
			int surfaceNum;
			read_binary_from_file(file, &surfaceNum);

			surface.resize(surfaceNum);
			for (int i = 0; i < surfaceNum; i++) {
				read_binary_from_file(file, &surface[i], header.m_data[pmx_header::VERTEX_INDEX_SIZE]);
			}

			
			//for (int i = 0; i < 10; i++) {
				//std::cout << surface[i].m_vertex_index << "\n";
			//}
			
		}

		//�e�N�X�`���p�X
		//�ǂݍ��߂ē��邯�ǃt�H���_�̋�؂肪�Ԉ���Ă��邩��
		std::vector<std::wstring> texturePaths{};
		{
			std::string folderPathBase{ fileName };
			folderPathBase = std::string{ folderPathBase.begin(),folderPathBase.begin() + folderPathBase.rfind('/') + 1 };
			std::wstring folderPath = to_wstring(folderPathBase);

			int textureNum;
			read_binary_from_file(file, &textureNum);

			texturePaths.reserve(textureNum);

			wchar_t wBuffer[512];
			int textSize;
			for (int i = 0; i < textureNum; i++) {

				read_binary_from_file(file, &textSize);
				read_binary_from_file(file, &wBuffer, textSize);
				texturePaths.emplace_back(folderPath + std::wstring{ &wBuffer[0], &wBuffer[0] + textSize / 2 });
			}
		}
		


		

		return true;
	}


}

