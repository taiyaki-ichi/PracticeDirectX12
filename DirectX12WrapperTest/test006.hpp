#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/TextureResource.hpp"
#include"Resource/ShaderResource.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"Resource/IndexBufferResource.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

namespace test006
{
	struct Vertex {
		float x, y, z;
		float u, v;
	};

	int main()
	{
		using namespace DX12;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		CommandList commandList{};
		commandList.Initialize(&device);

		Shader cs{};
		cs.Intialize(L"Shader/ComputeShader001.hlsl", "main", "cs_5_0");

		RootSignature computeRootsignature{};
		computeRootsignature.Initialize(&device,
			{ {DescriptorRangeType::SRV,DescriptorRangeType::UAV} },
			{}
		);

		PipelineState computePipelineState{};
		computePipelineState.Initialize(&device, &computeRootsignature, &cs);

		int textureWidth, textureHeight, n;
		//std::uint8_t* data = stbi_load("../../Assets/icon.png", &textureWidth, &textureHeight, &n, 0);
		std::uint8_t* data = stbi_load("../../Assets/1008010101.png", &textureWidth, &textureHeight, &n, 0);

		TextureResource textureResource{};
		textureResource.Initialize(&device, &commandList, data, textureWidth, textureHeight, textureWidth * n);

		stbi_image_free(data);

		Float4ShaderResource float4ShaderResource{};
		float4ShaderResource.Initialize(&device, textureWidth, textureHeight, { { 0.f,0.f,0.f,0.f } });

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeDescriptorHeap{};
		computeDescriptorHeap.Initialize(&device, 2);
		computeDescriptorHeap.PushBackView(&device, &textureResource);
		computeDescriptorHeap.PushBackUnorderedAccessView(&device, &float4ShaderResource);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> clearDescriptorHeap{};
		clearDescriptorHeap.Initialize(&device, 1);
		clearDescriptorHeap.PushBackView(&device, &float4ShaderResource);
	
		commandList.Barrior(&float4ShaderResource, ResourceState::UnorderedAccessResource);
		commandList.Get()->SetComputeRootSignature(computeRootsignature.Get());
		commandList.SetDescriptorHeap(&computeDescriptorHeap);
		commandList.Get()->SetComputeRootDescriptorTable(0, computeDescriptorHeap.GetGPUHandle());
		commandList.Get()->SetPipelineState(computePipelineState.Get());
		commandList.Get()->Dispatch(textureWidth, textureHeight, 1);
		commandList.Barrior(&float4ShaderResource, ResourceState::PixcelShaderResource);
		




		DoubleBuffer doubleBuffer{};
		auto [factry, swapChain] = commandList.CreateFactryAndSwapChain(hwnd);
		doubleBuffer.Initialize(&device, factry, swapChain);

		std::array<Vertex, 4> vertex{ {
			{-0.8f,-0.8f,0.f,0.f,1.f},
			{-0.8f,0.8f,0.f,0.f,0.f},
			{0.8f,-0.8f,0.f,1.f,1.f},
			{0.8f,0.8f,0.f,1.f,0.f}
			} };

		std::array<std::uint16_t, 6> index{
			0,1,2,2,1,3
		};

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(vertex), sizeof(decltype(vertex)::value_type));
		vertexBufferResource.Map(vertex);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(index));
		indexBufferResource.Map(index);

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 1);

		descriptorHeap.PushBackView(&device, &float4ShaderResource);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::SRV} }, { StaticSamplerType::Standard });

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, &vertexShader, &pixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3} ,{"TEXCOOD",VertexLayoutFormat::Float2} },
			{ Format::R8G8B8A8 }, false);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		while (UpdateWindow()) {
	

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);
			commandList.ClearBackBuffer(&doubleBuffer);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle());

			commandList.SetPipelineState(&pipelineState);

			commandList.SetDescriptorHeap(&descriptorHeap);
			commandList.SetRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			commandList.SetVertexBuffer(&vertexBufferResource);
			commandList.SetIndexBuffer(&indexBufferResource);
			commandList.DrawIndexedInstanced(6);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}


}