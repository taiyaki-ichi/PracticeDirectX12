#pragma once
#include<DirectXMath.h>
#include<stdio.h>
#include<iostream>
#include<string>
#include<vector>
#include<variant>
#include<string>
#include<codecvt>
#include<locale>

namespace ichi
{
	//文字列変換用
	using convert_t = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_t, wchar_t> strconverter;

	std::string to_string(std::wstring wstr)
	{
		return strconverter.to_bytes(wstr);
	}

	std::wstring to_wstring(std::string str)
	{
		return strconverter.from_bytes(str);
	}

	//pmx_headerのdataのインデックス
	enum header_data_index
	{
		ENCODING_FORMAT,
		NUMBER_OF_ADD_UV,
		VERTEX_INDEX_SIZE,
		TEXTURE_INDEX_SIZE,
		MATERIAL_INDEX_SIZE,
		BONE_INDEX_SIZE,
		RIGID_BODY_INDEX_SIZE
	};

#pragma pack(1)
	struct pmx_header 
	{
		//"Pmd "スペース注意
		unsigned char m_magic[4];
		//バージョン
		float m_version;
		//バイトサイズ
		unsigned char m_data_size;
		//バイト列
		unsigned char m_data[8];
	};
#pragma pack()

	//CharTypeはcharかwchar_tを想定
	//pmd_header.m_byte[0]が0なら文字コードはUTF16なのでwchar_t
	//1ならUTF8なのでchar
	template<typename CharType>
	struct pmx_model_info
	{
		//それぞれ最大値はとりあえず
		static constexpr size_t MAX_SIZE = 256;

		CharType m_model_name[MAX_SIZE];
		CharType m_model_name_eng[MAX_SIZE];
		CharType m_comment[MAX_SIZE];
		CharType m_comment_eng[MAX_SIZE];
	};

	//ボーンのウェイト
	struct pmx_born_weight_type_BDEF1 {
		int m_born[1];
	};
	struct pmx_born_weight_type_BDEF2 {
		int m_born[2];
		float m_weight[1];
	};
	struct pmx_born_weight_type_BDEF4 {
		int m_born[4];
		float m_weight[4];
	};
	struct pmx_born_weight_type_SDEF {
		int m_born[2];
		float m_weight[1];
		DirectX::XMFLOAT3 m_SDEF_vector[3];
	};

	//頂点情報
	struct pmx_vertex
	{
		//頂点座標
		DirectX::XMFLOAT3 m_position;
		//法線
		DirectX::XMFLOAT3 m_normal;
		//UV座標
		DirectX::XMFLOAT2 m_uv;
		//追加UV
		std::vector<DirectX::XMFLOAT4> m_additional_uv;

		//ボーンウェイト
		std::variant<pmx_born_weight_type_BDEF1, pmx_born_weight_type_BDEF2,
			pmx_born_weight_type_BDEF4, pmx_born_weight_type_SDEF> m_born_weight;

		//エッジ倍率
		float m_edge_magnification;
	};
	
	enum vertex_weight_type
	{
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF
	};

	//面！
	struct pmx_surface {
		int m_vertex_index;
	};

	inline bool load_pmx(const char* fileName) {
	//inline std::vector<DirectX::XMFLOAT3> load_pmx(const char* fileName) {
	
		FILE* file = fopen(fileName, "rb");
		
		if (!file) {
			std::cout << "fopen is failed\n";
			//return false;
		}


		//ヘッダ
		pmx_header header{};
		{
			fread(&header, sizeof(header), 1, file);

			/*
			std::cout << header.m_magic << "\n";
			std::cout << header.m_version << "\n";
			std::cout << header.m_data_size << "\n";
			for (size_t i = 0; i < 8; i++)
				std::cout << static_cast<unsigned int>(header.m_data[i]) << "\n";
				*/
		}
			

		//モデルの情報
		pmx_model_info<wchar_t> modelInfo{};
		{
			for (unsigned int i = 0; i < 4; i++) {
				unsigned int num;
				fread(&num, sizeof(num), 1, file);
				switch (i)
				{
				case 0:
					fread(&modelInfo.m_model_name, num, 1, file);
					break;
				case 1:
					fread(&modelInfo.m_model_name_eng, num, 1, file);
					break;
				case 2:
					fread(&modelInfo.m_comment, num, 1, file);
					break;
				case 3:
					fread(&modelInfo.m_comment_eng, num, 1, file);
					break;
				}
			}

			/*
			std::cout << modelInfo.m_model_name << "\n";
			std::cout << modelInfo.m_model_name_eng << "\n";
			std::cout << modelInfo.m_comment << "\n";
			std::cout << modelInfo.m_comment_eng << "\n";
			*/
		}


		//頂点っ情報
		std::vector<pmx_vertex> vertex{};
		{
			int vertexNum;
			fread(&vertexNum, sizeof(vertexNum), 1, file);

			//std::cout << vertexNum << "\n";

			vertex.reserve(vertexNum);

			pmx_vertex tmpVertex;
			pmx_born_weight_type_BDEF1 tmpBDEF1{};
			pmx_born_weight_type_BDEF2 tmpBDEF2{};
			pmx_born_weight_type_BDEF4 tmpBDEF4{};
			pmx_born_weight_type_SDEF tmpSDEF{};
			for (int i = 0; i < vertexNum; i++)
			{
				fread(&tmpVertex.m_position, sizeof(tmpVertex.m_position), 1, file);
				fread(&tmpVertex.m_normal, sizeof(tmpVertex.m_normal), 1, file);
				fread(&tmpVertex.m_uv, sizeof(tmpVertex.m_uv), 1, file);
				if (header.m_data[NUMBER_OF_ADD_UV] != 0) {
					auto num = header.m_data[NUMBER_OF_ADD_UV];
					tmpVertex.m_additional_uv.resize(num);
					fread(&tmpVertex.m_additional_uv, sizeof(tmpVertex.m_additional_uv[0]), num, file);
				}

				unsigned char weightMethod;
				fread(&weightMethod, sizeof(weightMethod), 1, file);

				switch (weightMethod)
				{
				case vertex_weight_type::BDEF1:
					fread(&tmpBDEF1.m_born, header.m_data[BONE_INDEX_SIZE], 1, file);
					tmpVertex.m_born_weight = std::move(tmpBDEF1);
					break;

				case vertex_weight_type::BDEF2:
					fread(&tmpBDEF2.m_born, header.m_data[BONE_INDEX_SIZE], 2, file);
					fread(&tmpBDEF2.m_weight, sizeof(&tmpBDEF2.m_weight[0]), 1, file);
					tmpVertex.m_born_weight = std::move(tmpBDEF2);
					break;

				case vertex_weight_type::BDEF4:
					fread(&tmpBDEF4.m_born, header.m_data[BONE_INDEX_SIZE], 4, file);
					fread(&tmpBDEF4.m_weight, sizeof(&tmpBDEF4.m_weight[0]), 4, file);
					tmpVertex.m_born_weight = std::move(tmpBDEF4);
					break;

				case vertex_weight_type::SDEF:
					fread(&tmpSDEF.m_born, header.m_data[BONE_INDEX_SIZE], 2, file);
					fread(&tmpSDEF.m_weight, sizeof(tmpSDEF.m_weight[0]), 1, file);
					fread(&tmpSDEF.m_SDEF_vector, sizeof(tmpSDEF.m_SDEF_vector[0]), 3, file);
					tmpVertex.m_born_weight = std::move(tmpSDEF);
					break;
				}

				fread(&tmpVertex.m_edge_magnification, 4, 1, file);

				vertex.emplace_back(std::move(tmpVertex));
			}

			//for (int i = 0; i < 10; i++) {
				//std::cout << vertex[i].m_position.x << "," << vertex[i].m_position.y << "," << vertex[i].m_position.z << "\n";
			//}
			/*
			std::vector<DirectX::XMFLOAT3> vert{};
			vert.reserve(vertex.size());
			for (int i = 0; i < vertex.size(); i++)
				vert.emplace_back(std::move(vertex[i].m_position));

			return vert;
			*/
		}


		//面
		std::vector<pmx_surface> surface{};
		{
			int surfaceNum;
			fread(&surfaceNum, sizeof(surfaceNum), 1, file);


			surface.resize(surfaceNum);
			for (int i = 0; i < surfaceNum; i++) {
				fread(&surface[i], header.m_data[VERTEX_INDEX_SIZE], 1, file);
			}

			/*
			for (int i = 0; i < 10; i++) {
				std::cout << surface[i].m_vertex_index << "\n";
			}
			*/
		}

		//テクスチャパス
		std::vector<std::wstring> texturePaths;
		{
			std::string folderPathBase{ fileName };
			//std::cout << folderPathBase.rfind('/') << "\n";
			folderPathBase = std::string{ folderPathBase.begin(),folderPathBase.begin() + folderPathBase.rfind('/') + 1 };
			std::wstring folderPath = to_wstring(folderPathBase);

			int textureNum;
			fread(&textureNum, sizeof(textureNum), 1, file);

			texturePaths.reserve(textureNum);

			wchar_t wBuffer[152];
			int textSize;
			for (int i = 0; i < textureNum; i++) {

				fread(&textSize, sizeof(textSize), 1, file);
				fread(&wBuffer, textSize, 1, file);
				texturePaths.emplace_back(folderPath + std::wstring{ &wBuffer[0], &wBuffer[0] + textSize / 2 });
			}

			/*
			for (auto& t : texturePaths) {
				std::wcout << t << "\n";
			}
			*/

		}



		

		return true;
	}


}