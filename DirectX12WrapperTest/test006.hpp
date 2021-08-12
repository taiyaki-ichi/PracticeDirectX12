#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/ShaderResource.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/VertexBuffer.hpp"
#include"Resource/IndexBuffer.hpp"


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

		Command command{};
		command.Initialize(&device);

		auto swapChain = command.CreateSwapChain(&device, hwnd);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> rtvDescriptorHeap{};
		rtvDescriptorHeap.Initialize(&device, 2);
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(0));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(1));


		Shader cs{};
		cs.Intialize(L"Shader/ComputeShader001.hlsl", "main", "cs_5_0");

		RootSignature computeRootsignature{};
		computeRootsignature.Initialize(&device,
			{ {DescriptorRangeType::SRV,DescriptorRangeType::UAV} },
			{}
		);

		PipelineState computePipelineState{};
		computePipelineState.Initialize(&device, &computeRootsignature, &cs);

		int x, y, n;
		ShaderResource textureResource{};
		{
			std::uint8_t* data = stbi_load("../../Assets/icon.png", &x, &y, &n, 0);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(x * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * y);
			uploadResource.Map(data, x * 4, y);
			textureResource.Initialize(&device, x, y, { Type::UnsignedNormalizedFloat,4 }, 1);

			command.Reset(0);
			command.Barrior(&textureResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &textureResource);
			command.Barrior(&textureResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		ShaderResource float4ShaderResource{};
		float4ShaderResource.Initialize(&device, x, y, { Type::UnsignedNormalizedFloat,4 }, 1, { { 0.f,0.f,0.f,0.f } });

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeDescriptorHeap{};
		computeDescriptorHeap.Initialize(&device, 2);
		computeDescriptorHeap.PushBackView(&device, &textureResource);
		computeDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &float4ShaderResource);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> clearDescriptorHeap{};
		clearDescriptorHeap.Initialize(&device, 1);
		clearDescriptorHeap.PushBackView(&device, &float4ShaderResource);


		//
		command.Reset(0);
		command.Barrior(&float4ShaderResource, ResourceState::UnorderedAccessResource);
		command.SetComputeRootSignature(&computeRootsignature);
		command.SetDescriptorHeap(&computeDescriptorHeap);
		command.SetComputeRootDescriptorTable(0, computeDescriptorHeap.GetGPUHandle());
		command.SetPipelineState(&computePipelineState);
		command.Barrior(&float4ShaderResource, ResourceState::PixcelShaderResource);
		command.Dispatch(x, y, 1);
		command.Close();
		command.Execute();
		command.Fence(0);
		command.Wait(0);
	


		std::array<Vertex, 4> vertex{ {
			{-0.8f,-0.8f,0.f,0.f,1.f},
			{-0.8f,0.8f,0.f,0.f,0.f},
			{0.8f,-0.8f,0.f,1.f,1.f},
			{0.8f,0.8f,0.f,1.f,0.f}
			} };

		std::array<std::uint32_t, 6> index{
			0,1,2,2,1,3
		};

		VertexBuffer vertexBuffer{};
		vertexBuffer.Initialize(&device, vertex.size(), sizeof(decltype(vertex)::value_type));
		vertexBuffer.Map(vertex);

		IndexBuffer indexBuffer{};
		indexBuffer.Initialize(&device, index.size());
		indexBuffer.Map(index);

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
		pipelineState.Initialize(&device, &rootSignature, { &vertexShader, &pixelShader },
			{ {"POSITION", {Type::Float,3}} ,{"TEXCOOD",{Type::Float,2}} },
			{ {Type::UnsignedNormalizedFloat,4} }, false, false, PrimitiveTopology::Triangle
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		while (UpdateWindow()) {

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex));

			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);

			command.SetGraphicsRootSignature(&rootSignature);
			command.SetDescriptorHeap(&descriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			command.SetVertexBuffer(&vertexBuffer);
			command.SetIndexBuffer(&indexBuffer);
			command.DrawIndexedInstanced(6);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::Common);

			command.Close();
			command.Execute();
			
			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());
		}
		command.WaitAll(&device);

		return 0;
	}


}