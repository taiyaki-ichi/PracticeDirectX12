#pragma once
#include<DirectXMath.h>
#include<vector>
#include<variant>

namespace MMDL
{

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

		//バージョン
		float m_version{};
		//バイトサイズ
		unsigned char m_data_size{};
		//バイト列
		unsigned char m_data[8]{};
	};

	//CharTypeはcharかwchar_tを想定
	//pmd_header.m_byte[0]が0なら文字コードはUTF16なのでwchar_t
	//1ならUTF8なのでchar
	template<typename StringType>
	struct pmx_info
	{
		//それぞれ最大値はとりあえず
		static constexpr size_t MAX_SIZE = 256;

		enum data_index {
			MODEL_NAME,
			MODEL_NAME_ENG,
			COMMENT,
			COMMENT_ENG
		};

		StringType m_data[4]{};
	};


	//頂点情報
	struct pmx_vertex
	{

		//頂点座標
		DirectX::XMFLOAT3 m_position{};
		//法線
		DirectX::XMFLOAT3 m_normal{};
		//UV座標
		DirectX::XMFLOAT2 m_uv{};
		//追加UV
		std::vector<DirectX::XMFLOAT4> m_additional_uv{};

		//ボーンウェイトとか
		//とりあえず配列で実装、不便だったらカエル
		//初期値はpmxEditor見て決めた、不適切ならカエル
		int m_born[4]{ -1,-1,-1,-1 };
		float m_weight[4]{ 0.f,0.f,0.f,0.f };
		DirectX::XMFLOAT3 m_SDEF_vector[3]{ {0.f,0.f,0.f},{0.f,0.f,0.f},{0.f,0.f,0.f} };

		//エッジ倍率
		float m_edge_magnification{};
	};

	//面
	struct pmx_surface {
		int m_vertex_index = 0;
	};

	//マテリアル
	template<typename StringType>
	struct pmx_material
	{
		StringType m_name{};
		StringType m_name_eng{};

		DirectX::XMFLOAT4 m_diffuse{};
		DirectX::XMFLOAT3 m_specular{};
		float m_specularity = 0.f;
		DirectX::XMFLOAT3 m_ambient{};

		//描画フラグ
		//左のbitから、両面描画、地面影、セルフシャドウマップへの描画
		//セルフシャドウの描画、エッジ描画、の順
		unsigned char m_drawing_flag = 0;

		//エッジ色(R,G,B,A)
		DirectX::XMFLOAT4 m_edge_color{};
		//エッジサイズ
		float m_edge_size = 0;

		//通常テクスチャ、テクスチャテーブルの参照Index
		int m_texture_index_size_1 = -1;
		//スフィアテクスチャ、テクスチャテーブルの参照Index（テクスチャの拡張子の制限なし）
		int m_texture_index_size_2 = -1;
		//スフィアモード
		//０：無向　１：乗算　２：加算　３：サブテクスチャ
		unsigned char m_sphere_mode = 0;;

		//unsigned intなら個別toon、unsigned charなら共有toon
		//一応とりあえず、型で識別できるようにしておく
		std::variant<unsigned int, unsigned char> m_toon{};

		//メモ、自由欄
		StringType m_memo{};

		//材質に対応する面（頂点）数（必ず3の倍数）
		int m_vertex_num = -1;

	};

	//ボーン
	template<typename StringType>
	struct pmx_born
	{
		StringType m_name{};
		StringType m_name_eng{};

		//位置
		DirectX::XMFLOAT3 m_position{};
		//親ボーンのインデックス
		int m_parent_index = -1;
		//変形階層
		int m_transformation_level = -1;;

		//ボーンフラグ
		unsigned short m_flag = 0;;

		//座標のおっふせっと、ボーンの位置からの相対分
		DirectX::XMFLOAT3 m_offset{};
		//接続先ビーンのボーンIndex
		int m_children_index = -1;;
		//付与親ボーンのボーンIndex
		int m_impart_parent_index = -1;
		//付与率
		float m_impart_rate = 0.f;
		//固定軸の方向ベクトル
		DirectX::XMFLOAT3 m_fixed_axis{};
		//ローカルのX軸の方向ベクトル
		DirectX::XMFLOAT3 m_local_axis_x{};
		//ローカルのZ軸の方向ベクトル
		DirectX::XMFLOAT3 m_local_axis_y{};
		//外部親変形
		int m_external_parent_key = -1;

		//IKターゲットボーンのボーンIndex
		int m_IK_target_index = -1;
		//IKループ回数
		int m_IK_loop_count = -1;
		//IKるうーぷ計算時の1回当たりの制限角度
		float m_IK_unit_angle = 0.f;
		struct IK_link {
			int m_index = -1;
			unsigned char m_exist_angle_limited = 0;
			DirectX::XMFLOAT3 m_limit_angle_min{};
			DirectX::XMFLOAT3 m_limit_angle_max{};
		};
		std::vector<IK_link> m_IK_link{};

	};


	//全部まとめたやつ
	template<typename StringType>
	struct pmx_model
	{
		pmx_header m_header;
		pmx_info<StringType> m_info;
		std::vector<pmx_vertex> m_vertex;
		std::vector<pmx_surface> m_surface;
		std::vector<StringType> m_texture_path;
		std::vector<pmx_material<StringType>> m_material;
		std::vector<pmx_born<StringType>> m_born;
	};

}