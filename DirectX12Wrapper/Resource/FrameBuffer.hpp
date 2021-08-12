#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	//SwapChain���Ő���
	class FrameBuffer : public ResourceBase {};

	template<>
	struct DefaultViewTypeTraits<FrameBuffer>
	{
		using Type = DescriptorHeapViewTag::ShaderResource;
	};

}