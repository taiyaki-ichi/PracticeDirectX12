#pragma once
#include"include/pmx_data_struct.hpp"
#include<DirectXMath.h>
#include<algorithm>
#include<iterator>

namespace ichi
{

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

	//my_vertex�̐���
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