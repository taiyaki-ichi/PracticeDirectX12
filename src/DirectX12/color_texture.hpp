#pragma once
#include"resource_type_tag.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	ID3D12Resource* create_white_texture(device*);
	ID3D12Resource* create_black_texture(device*);
	ID3D12Resource* create_gray_gradation_texture(device*);

	//4Å~4ÇÃîí
	class white_texture_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		using resource_type = shader_resource_tag;
		using create_view_type = typename create_view_type::SRV;

		white_texture_resource() = default;
		~white_texture_resource();

		bool initialize(device*);

		ID3D12Resource* get();
	};

	class black_texture_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		using resource_type = shader_resource_tag;
		using create_view_type = typename create_view_type::SRV;

		black_texture_resource() = default;
		~black_texture_resource();

		bool initialize(device*);

		ID3D12Resource* get();
	};

	class gray_gradation_texture_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		using resource_type = shader_resource_tag;
		using create_view_type = typename create_view_type::SRV;

		gray_gradation_texture_resource() = default;
		~gray_gradation_texture_resource();

		bool initialize(device*);

		ID3D12Resource* get();
	};

}