#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/ViewTypeTag.hpp"

namespace DX12
{
	//SwapChain���Ő���
	class FrameBuffer : public ResourceBase {};

	template<>
	struct DefaultViewTypeTraits<FrameBuffer>
	{
		using Type = ViewTypeTag::ShaderResource;
	};

}