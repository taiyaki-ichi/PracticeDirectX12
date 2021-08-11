#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	//ダブルバッファを作る時に使用
	class DoubleBufferResource : public ResourceBase {};

	template<>
	struct DefaultViewTypeTraits<DoubleBufferResource>
	{
		using Type = DescriptorHeapViewTag::ShaderResource;
	};

}