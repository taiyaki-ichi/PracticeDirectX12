#pragma once
#include"../include/pmx_data_struct.hpp"
#include"utility.hpp"
#include<tuple>
#include<fstream>

namespace ichi
{

	//読み込むときに使用
	enum pmx_vertex_weight_type
	{
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF
	};

	enum bone_flag_mask
	{
		ACCESS_POINT = 0x0001,
		IK = 0x0020,
		IMPART_TRANSLATION = 0x0100,
		IMPART_ROTATION = 0x0200,
		AXIS_FIXING = 0x0400,
		LOCAL_AXIS = 0x0800,
		EXTERNAL_PARENT_TRANS = 0x2000,
	};


	namespace {
		//load_pmx_implの戻り値
		template<typename StringType>
		using ResultType = std::tuple<
			pmx_info<StringType>,
			std::vector<pmx_vertex>,
			std::vector<pmx_surface>, 
			std::vector<StringType>, 
			std::vector<pmx_material<StringType>>,
			std::vector<pmx_born<StringType>>
		>;
	}

	//文字コードによって処理を分離させたい
	//ト、思ったけどインターフェース側の戻り値どうするかな
	//文字列の型が違うだけだからvariant使うかな
	template<typename StringType>
	inline ResultType<StringType> load_pmx_impl(std::ifstream& file,const pmx_header& header,const char* fileName)
	{
		//
		//モデルの情報
		//
		pmx_info<StringType> modelInfo{};
		{
			unsigned int num;
			for (unsigned int i = 0; i < 4; i++) {
				read_binary_from_file(file, &num);
				modelInfo.m_data[i].resize(num);
				read_binary_from_file(file, &modelInfo.m_data[i][0], num);
			}
		}

		//
		//頂点情報
		//
		std::vector<pmx_vertex> vertex{};
		{
			int vertexNum;
			read_binary_from_file(file, &vertexNum);

			vertex.resize(vertexNum);

			for (int i = 0; i < vertexNum; i++)
			{
				read_binary_from_file(file, &vertex[i].m_position);
				read_binary_from_file(file, &vertex[i].m_normal);
				read_binary_from_file(file, &vertex[i].m_uv);

				if (header.m_data[pmx_header::NUMBER_OF_ADD_UV] != 0) {
					auto num = header.m_data[pmx_header::NUMBER_OF_ADD_UV];
					vertex[i].m_additional_uv.resize(num);
					size_t size = header.m_data[pmx_header::NUMBER_OF_ADD_UV] * sizeof(vertex[i].m_additional_uv[0]);
					read_binary_from_file(file, &vertex[i].m_additional_uv, size);
				}

				unsigned char weightMethod;
				read_binary_from_file(file, &weightMethod);

				//取得したウェイトのタイプの有効なデータを取得していく
				switch (weightMethod)
				{
				case pmx_vertex_weight_type::BDEF1:
					read_binary_from_file(file, &vertex[i].m_born, header.m_data[pmx_header::BONE_INDEX_SIZE]);
					break;

				case pmx_vertex_weight_type::BDEF2:
					read_binary_from_file(file, &vertex[i].m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 2);
					read_binary_from_file(file, &vertex[i].m_weight, sizeof(&vertex[i].m_weight[0]));
					break;

				case pmx_vertex_weight_type::BDEF4:
					read_binary_from_file(file, &vertex[i].m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 4);
					read_binary_from_file(file, &vertex[i].m_weight, sizeof(&vertex[i].m_weight[0]) * 4);
					break;

				case pmx_vertex_weight_type::SDEF:
					read_binary_from_file(file, &vertex[i].m_born, header.m_data[pmx_header::BONE_INDEX_SIZE] * 2);
					read_binary_from_file(file, &vertex[i].m_weight, sizeof(vertex[i].m_weight[0]));
					read_binary_from_file(file, &vertex[i].m_SDEF_vector, sizeof(vertex[i].m_SDEF_vector[0]) * 3);
					break;
				}

				read_binary_from_file(file, &vertex[i].m_edge_magnification);

			}
		}


		//
		//面
		//
		std::vector<pmx_surface> surface{};
		{
			int surfaceNum;
			read_binary_from_file(file, &surfaceNum);

			surface.resize(surfaceNum);
			for (int i = 0; i < surfaceNum; i++) {
				read_binary_from_file(file, &surface[i], header.m_data[pmx_header::VERTEX_INDEX_SIZE]);
			}
		}

		//
		//テクスチャパス
		//読み込めて入るけどフォルダの区切りが間違っているかも
		//
		std::vector<StringType> texturePaths{};
		{
			std::string folderPathBase{ fileName };
			folderPathBase = std::string{ folderPathBase.begin(),folderPathBase.begin() + folderPathBase.rfind('/') + 1 };

			StringType folderPath{};
			if constexpr (std::is_same_v<StringType, std::wstring>) {
				folderPath = to_wstring(std::move(folderPathBase));
			}
			else if constexpr (std::is_same_v<StringType, std::string>) {
				folderPath = std::move(folderPathBase);
			}

			//std::wstring folderPath = to_wstring(folderPathBase);

			int textureNum;
			read_binary_from_file(file, &textureNum);

			texturePaths.reserve(textureNum);

			wchar_t wBuffer[512];
			int textSize;
			for (int i = 0; i < textureNum; i++) {

				read_binary_from_file(file, &textSize);
				read_binary_from_file(file, &wBuffer, textSize);
				texturePaths.emplace_back(folderPath + StringType{ &wBuffer[0], &wBuffer[0] + textSize / 2 });
			}
		}

		//
		//マテリアル
		//
		std::vector<pmx_material<StringType>> material{};
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


		//
		//ボーン
		//
		std::vector<pmx_born<StringType>> bone{};
		{
			int boneNum;
			read_binary_from_file(file, &boneNum);

			bone.resize(boneNum);

			int size;

			for (int i = 0; i < boneNum; i++)
			{
				//名前
				read_binary_from_file(file, &size);
				bone[i].m_name.resize(size);
				read_binary_from_file(file, &bone[i].m_name[0], size);
				read_binary_from_file(file, &size);
				bone[i].m_name_eng.resize(size);
				read_binary_from_file(file, &bone[i].m_name_eng[0], size);

				read_binary_from_file(file, &bone[i].m_position);
				read_binary_from_file(file, &bone[i].m_parent_index, header.m_data[pmx_header::data_index::BONE_INDEX_SIZE]);
				if (boneNum <= bone[i].m_parent_index)
					bone[i].m_parent_index = -1;
				read_binary_from_file(file, &bone[i].m_transformation_level);
				read_binary_from_file(file, &bone[i].m_flag);

				if (bone[i].m_flag & bone_flag_mask::ACCESS_POINT)
				{
					read_binary_from_file(file, &bone[i].m_children_index, header.m_data[pmx_header::data_index::BONE_INDEX_SIZE]);
				}
				else
				{
					read_binary_from_file(file, &bone[i].m_offset);
				}
				if ((bone[i].m_flag & bone_flag_mask::IMPART_TRANSLATION) ||
					(bone[i].m_flag & bone_flag_mask::IMPART_ROTATION))
				{
					read_binary_from_file(file, &bone[i].m_impart_parent_index, header.m_data[pmx_header::data_index::BONE_INDEX_SIZE]);
					read_binary_from_file(file, &bone[i].m_impart_rate);
				}
				if (bone[i].m_flag & bone_flag_mask::AXIS_FIXING)
				{
					read_binary_from_file(file, &bone[i].m_fixed_axis);
				}
				if (bone[i].m_flag & bone_flag_mask::LOCAL_AXIS)
				{
					read_binary_from_file(file, &bone[i].m_local_axis_x);
					read_binary_from_file(file, &bone[i].m_local_axis_y);
				}
				if (bone[i].m_flag & bone_flag_mask::EXTERNAL_PARENT_TRANS)
				{
					read_binary_from_file(file, &bone[i].m_external_parent_key);
				}
				if (bone[i].m_flag & bone_flag_mask::IK)
				{
					read_binary_from_file(file, &bone[i].m_IK_target_index, header.m_data[pmx_header::data_index::BONE_INDEX_SIZE]);
					read_binary_from_file(file, &bone[i].m_IK_loop_count);
					read_binary_from_file(file, &bone[i].m_IK_unit_angle);
					read_binary_from_file(file, &size);

					bone[i].m_IK_link.resize(size);

					for (int j = 0; j < size; j++)
					{
						read_binary_from_file(file, &bone[i].m_IK_link[j].m_index, header.m_data[pmx_header::data_index::BONE_INDEX_SIZE]);
						read_binary_from_file(file, &bone[i].m_IK_link[j].m_exist_angle_limited);
						if (bone[i].m_IK_link[j].m_exist_angle_limited == 1)
						{
							read_binary_from_file(file, &bone[i].m_IK_link[j].m_limit_angle_min);
							read_binary_from_file(file, &bone[i].m_IK_link[j].m_limit_angle_max);
						}

					}

				}

			}

		}


		return { modelInfo,vertex,surface,texturePaths,material,bone };
	}


}