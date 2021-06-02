#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	//�_�u���o�b�t�@����鎞�Ɏg�p
	class DoubleBufferResource : public ResourceBase {};

	template<>
	struct ViewTypeTraits<DoubleBufferResource>
	{
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

}