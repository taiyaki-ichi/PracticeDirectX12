#pragma once
#include"resource_type_tag.hpp"
#include"resource.hpp"
#include"device.hpp"
#include<vector>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	
	//カラーテクスチャのベースとなるクラス
	class color_texture_resource_base : public resource
	{
	public:
		virtual ~color_texture_resource_base() = default;

		bool initialize(device*, unsigned int width, unsigned int height);
	};

	//4*4の単色のリソース
	template<unsigned char Color>
	class simple_color_texture_resource : public color_texture_resource_base
	{
	public:
		using resource_type = typename resource_type::SRV;

		bool initialize(device* device);
	};

	using white_texture_resource = simple_color_texture_resource<0xff>;
	using black_texture_resource = simple_color_texture_resource<0x00>;


	class gray_gradation_texture_resource : public color_texture_resource_base
	{
	public:
		using resource_type = typename resource_type::SRV;

		bool initialize(device*);
	};




	//
	//以下、simple_color_texture_resourceの定義
	//


	template<unsigned char Color>
	inline bool simple_color_texture_resource<Color>::initialize(device* device)
	{
		if (!color_texture_resource_base::initialize(device, 4, 4))
			return false;

		std::vector<unsigned char> data(4 * 4 * 4);
		std::fill(data.begin(), data.end(), static_cast<unsigned char>(Color));

		if (FAILED(get()->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<unsigned int>(data.size())))) {
			std::cout << "color write is failed\n";
			return false;
		}

		return true;
	}
	
}