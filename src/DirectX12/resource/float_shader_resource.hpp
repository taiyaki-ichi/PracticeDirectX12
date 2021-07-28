#pragma once
#include"resource_base.hpp"
#include"../descriptor_heap_type.hpp"

namespace DX12
{
	//float��̃f�[�^
	//�Ⴆ�Ζ��x�����������������Ɏg�p
	class float_shader_resource : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height, std::optional<D3D12_CLEAR_VALUE> clearValue = std::nullopt);
	};


	//�f�B�X�N���v�^�p
	template<>
	struct DefaultViewTypeTraits<float_shader_resource> {
		using view_type = view_type::float_shader_resource;
	};
	DefineGetResourcePtr(float_shader_resource)
}