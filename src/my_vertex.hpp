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
		std::vector<my_vertex> result{};

		auto func = [](const MMDL::pmx_vertex& v) -> my_vertex {
			return { v.m_position,v.m_normal,v.m_uv };
		};

		std::transform(vertex.begin(), vertex.end(), std::back_inserter(result), std::move(func));

		return result;
	}

	struct my_surface {
		unsigned short m_vertex_index;
	};

	struct my_material {
		//MMDL��material��3���Ⴀ�Ȃ���4������
		//�Ƃ肠������FLOAT3�ł��
		DirectX::XMFLOAT4 m_diffuse;
		DirectX::XMFLOAT3 m_specular;
		float m_specularity;
		DirectX::XMFLOAT3 m_ambient;
	};

	//my_material�̐���
	template<typename StringType>
	inline std::vector<my_material> generate_my_material(const std::vector<MMDL::pmx_material<StringType>>& material)
	{

		std::vector<my_material> result{};

		auto func = [](const auto& m) -> my_material {
			return { m.m_diffuse,m.m_specular,m.m_specularity,m.m_ambient };
		};

		std::transform(material.begin(), material.end(), std::back_inserter(result), std::move(func));

		return result;
	}


}