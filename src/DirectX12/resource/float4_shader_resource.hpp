#pragma once
#include"resource_base.hpp"

namespace DX12
{
	//RGBAのデータ
	//普通に色を扱いたい時とかに使用
	class float4_shader_resource : public resource_base
	{
	public:
		bool initialize(device*, unsigned int width, unsigned int height, std::optional<D3D12_CLEAR_VALUE> clearValue = std::nullopt);
	};
}