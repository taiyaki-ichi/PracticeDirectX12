#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	class FrameBufferResource : public ResourceBase {};

	template<>
	struct DefaultViewTypeTraits<FrameBufferResource>
	{
		using Type = DescriptorHeapViewTag::ShaderResource;
	};

}