#pragma once
#include"include/pmx_data_struct.hpp"
#include<DirectXMath.h>
#include<algorithm>
#include<iterator>

namespace ichi
{

	//pmx_vertexから必要な情報を抽出
	//随時、更新していく
	struct my_vertex
	{
		//頂点座標
		DirectX::XMFLOAT3 m_position{};
		//法線
		DirectX::XMFLOAT3 m_normal{};
		//UV座標
		DirectX::XMFLOAT2 m_uv{};
	};

	//my_vertexの生成
	inline std::vector<my_vertex> generate_my_vertex(const std::vector<MMDL::pmx_vertex>& vertex)
	{
		std::vector<my_vertex> result(vertex.size());

		auto func = [](const MMDL::pmx_vertex& v) -> my_vertex {
			return { v.m_position,v.m_normal,v.m_uv };
		};

		std::transform(vertex.begin(), vertex.end(), std::back_inserter(result), std::move(func));

		return result;
	}


}