#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/ViewTypeTag.hpp"

namespace DX12
{
	//SwapChainì‡Ç≈ê∂ê¨
	class FrameBuffer : public ResourceBase {};

	template<>
	struct DefaultViewTypeTraits<FrameBuffer>
	{
		using Type = ViewTypeTag::ShaderResource;
	};

}