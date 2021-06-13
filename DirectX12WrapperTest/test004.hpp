#pragma once
#include"Resource/ShaderResource.hpp"
#include"Resource/DepthStencilBufferResource.hpp"

namespace test004
{
	inline int main()
	{
		using namespace DX12;

		CubeMapShaderResource cubeMapShaderResource{};

		CubeMapDepthStencilBufferResource cubeMapDepthStencilBufferResource{};

		return 0;
	}
}