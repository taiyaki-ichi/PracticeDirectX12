#pragma once
#include"resource_base.hpp"
#include"../descriptor_heap_type.hpp"
#include<vector>

#include<iostream>

namespace DX12
{

	//�J���[�e�N�X�`���̃x�[�X�ƂȂ�N���X
	class color_texture_resource_base : public resource_base
	{
	public:
		virtual ~color_texture_resource_base() = default;

		bool initialize(device*, unsigned int width, unsigned int height);
	};

	//4*4�̒P�F�̃��\�[�X
	template<unsigned char Color>
	class simple_color_texture_resource : public color_texture_resource_base
	{
	public:
		bool initialize(device* device);
	};

	//�����e�N�X�`��
	using white_texture_resource = simple_color_texture_resource<0xff>;
	template<>
	struct DefaultViewTypeTraits<white_texture_resource> {
		using view_type = view_type::float4_shader_resource;
	};
	DefineGetResourcePtr(white_texture_resource)


	//�����e�N�X�`��
	using black_texture_resource = simple_color_texture_resource<0x00>;
	template<>
	struct DefaultViewTypeTraits<black_texture_resource> {
		using view_type = view_type::float4_shader_resource;
	};
	DefineGetResourcePtr(black_texture_resource)


	//�O���[�Ł[�����
	class gray_gradation_texture_resource : public color_texture_resource_base
	{
	public:
		bool initialize(device*);
	};

	template<>
	struct DefaultViewTypeTraits<gray_gradation_texture_resource> {
		using view_type = view_type::float4_shader_resource;
	};
	DefineGetResourcePtr(gray_gradation_texture_resource)




	//
	//�ȉ��Asimple_color_texture_resource�̒�`
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