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
		//dataのインデックス用
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
	template<typename StringType>
	struct pmx_model_info
	{
		//それぞれ最大値はとりあえず
		static constexpr size_t MAX_SIZE = 256;

		enum data_index {
			MODEL_NAME,
			MODEL_NAME_ENG,
			COMMENT,
			COMMENT_ENG
		};

		StringType m_data[4];
	};


	//頂点情報
	struct pmx_vertex
	{
		//ボーンのウェイト構造体s
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


		//頂点座標
		DirectX::XMFLOAT3 m_position;
		//法線
		DirectX::XMFLOAT3 m_normal;
		//UV座標
		DirectX::XMFLOAT2 m_uv;
		//追加UV
		std::vector<DirectX::XMFLOAT4> m_additional_uv;

		//ボーンウェイト
		std::variant<born_weight_type_BDEF1, born_weight_type_BDEF2,
			born_weight_type_BDEF4, born_weight_type_SDEF> m_born_weight;

		//エッジ倍率
		float m_edge_magnification;
	};
	
	//読み込むときに使用
	//読み込みだけにしか使わないっぽいからヘッダに置かなくても良いかな
	enum pmx_vertex_weight_type
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

	//マテリアル
	template<typename StringType>
	struct pmx_material
	{
		StringType m_name;
		StringType m_name_eng;

		DirectX::XMFLOAT4 m_diffuse;
		DirectX::XMFLOAT3 m_specular;
		float m_specularity;
		DirectX::XMFLOAT3 m_ambient;

		//描画フラグ
		//左のbitから、両面描画、地面影、セルフシャドウマップへの描画
		//セルフシャドウの描画、エッジ描画、の順
		unsigned char m_drawing_flag;

		//エッジ色(R,G,B,A)
		DirectX::XMFLOAT4 m_edge_color;
		//エッジサイズ
		float m_edge_size;

		//通常テクスチャ、テクスチャテーブルの参照Index
		unsigned int m_texture_index_size_1;
		//スフィアテクスチャ、テクスチャテーブルの参照Index（テクスチャの拡張子の制限なし）
		unsigned int m_texture_index_size_2;
		//スフィアモード
		//０：無向　１：乗算　２：加算　３：サブテクスチャ
		unsigned char m_sphere_mode;

		//unsigned intなら個別toon、unsigned charなら共有toon
		//一応、型で識別できるようにしておく
		std::variant<unsigned int, unsigned char> m_toon;
		//unsigned int m_toon;

		//メモ、自由欄
		StringType m_memo;

		//材質に対応する面（頂点）数（必ず3の倍数）
		int m_vertex_num;

	};

	inline bool load_pmx(const char* fileName) {
	//inline std::vector<DirectX::XMFLOAT3> load_pmx(const char* fileName) {

		//wcout使うときに必要
		std::wcout.imbue(std::locale(""));

		std::ifstream file{ fileName, std::ios::in | std::ios::binary };
		
		if (!file) {
			//std::cout << "fopen is failed\n";
			return false;
		}


		//ヘッダ
		pmx_header header{};
		{
			read_binary_from_file(file, &header);
			
			//std::cout << header.m_magic << "\n";
			//std::cout << header.m_version << "\n";
			//std::cout << header.m_data_size << "\n";
			//for (size_t i = 0; i < 8; i++)
				//std::cout << static_cast<unsigned int>(header.m_data[i]) << "\n";
				
		}
			

		//モデルの情報
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

		
		//頂点っ情報
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


		//面
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

		//テクスチャパス
		//読み込めて入るけどフォルダの区切りが間違っているかも
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
		
		

		//マテリアル
		std::vector<pmx_material<std::wstring>> material{};
		{	
			int materialNum;
			read_binary_from_file(file, &materialNum);

			material.resize(materialNum);

			for (int i = 0; i < materialNum; i++) {

				int size;

				//名前
				read_binary_from_file(file, &size);
				material[i].m_name.resize(size);
				read_binary_from_file(file, &material[i].m_name[0], size);
				read_binary_from_file(file, &size);
				material[i].m_name_eng.resize(size);
				read_binary_from_file(file, &material[i].m_name_eng[0], size);

				//それぞれ
				read_binary_from_file(file, &material[i].m_diffuse);
				read_binary_from_file(file, &material[i].m_specular);
				read_binary_from_file(file, &material[i].m_specularity);
				read_binary_from_file(file, &material[i].m_ambient);

				//描画フラグ
				read_binary_from_file(file, &material[i].m_drawing_flag);

				//エッジ
				read_binary_from_file(file, &material[i].m_edge_color);
				read_binary_from_file(file, &material[i].m_edge_size);

				read_binary_from_file(file, &material[i].m_texture_index_size_1, 
					header.m_data[pmx_header::data_index::TEXTURE_INDEX_SIZE]);
				read_binary_from_file(file, &material[i].m_texture_index_size_2,
					header.m_data[pmx_header::data_index::TEXTURE_INDEX_SIZE]);
				read_binary_from_file(file, &material[i].m_sphere_mode);

				unsigned char toonFlag;
				read_binary_from_file(file, &toonFlag);
				if (!toonFlag) {
					unsigned int foo;
					read_binary_from_file(file, &foo, header.m_data[pmx_header::data_index::TEXTURE_INDEX_SIZE]);
					material[i].m_toon = foo;
				}
				else {
					unsigned char foo;
					read_binary_from_file(file, &foo);
					material[i].m_toon = foo;
				}

				read_binary_from_file(file, &size);
				material[i].m_memo.resize(size);
				read_binary_from_file(file, &material[i].m_memo[0], size);

				read_binary_from_file(file, &material[i].m_vertex_num);
			}
		}

		//for (int i = 0; i < 5; i++) {
			//std::wcout << material[i].m_name << "\n";
		//}

		

		return true;
	}


}

