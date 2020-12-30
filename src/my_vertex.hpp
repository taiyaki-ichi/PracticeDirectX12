#pragma once
#include"include/pmx_data_struct.hpp"
#include<DirectXMath.h>
#include<algorithm>
#include<iterator>
#include"mmd.hpp"
#include<vector>

namespace ichi
{


	//my_vertex‚Ì¶¬
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



	//my_material‚Ì¶¬
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

	//my_material_info‚Ì¶¬
	template<typename StringType>
	inline std::vector<my_material_info> generate_my_material_info(const std::vector<MMDL::pmx_material<StringType>>& material)
	{

		std::vector<my_material_info> result{};

		auto func = [](const auto& m) -> my_material_info {
			return { m.m_texture_index_size_1,m.m_texture_index_size_2,m.m_vertex_num };
		};

		std::transform(material.begin(), material.end(), std::back_inserter(result), std::move(func));

		return result;
	}


}