#pragma once
#include"resource_base.hpp"

namespace DX12
{
	//float��̃f�[�^
	//�Ⴆ�Ζ��x�����������������Ɏg�p
	class float_shader_resource : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height, std::optional<D3D12_CLEAR_VALUE> clearValue = std::nullopt);
	};
}