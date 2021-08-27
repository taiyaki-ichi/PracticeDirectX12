#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/shader_resource.hpp"
#include"DescriptorHeap.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"Resource/map.hpp"

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

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(&device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV(&device, &swapChain.GetFrameBuffer(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV(&device, &swapChain.GetFrameBuffer(1), 0, 0);


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
		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> textureResource{};
		{
			std::uint8_t* data = stbi_load("../../Assets/icon.png", &x, &y, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(&device, TextureDataPitchAlignment(x * 4) * y);
			map(&uploadResource, data, x * 4, y, TextureDataPitchAlignment(x * 4));

			textureResource.initialize(&device, x, y, 1, 1);

			command.Reset(0);
			command.Barrior(&textureResource, resource_state::CopyDest);
			command.CopyTexture(&device, &uploadResource, &textureResource);
			command.Barrior(&textureResource, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>, resource_flag::AllowUnorderdAccess> float4ShaderResource{};
		float4ShaderResource.initialize(&device, x, y, 1, 0);

		descriptor_heap_CBV_SRV_UAV computeDescriptorHeap{};
		computeDescriptorHeap.initialize(&device, 2);
		computeDescriptorHeap.push_back_texture2D_SRV(&device, &textureResource, 1, 0, 0, 0.f);
		computeDescriptorHeap.push_back_texture2D_UAV(&device, &float4ShaderResource, 0, 0);


		//
		command.Reset(0);
		command.Barrior(&float4ShaderResource, resource_state::UnorderedAccessResource);
		command.SetComputeRootSignature(&computeRootsignature);
		command.SetDescriptorHeap(&computeDescriptorHeap);
		command.SetComputeRootDescriptorTable(0, computeDescriptorHeap.get_GPU_handle());
		command.SetPipelineState(&computePipelineState);
		command.Barrior(&float4ShaderResource, resource_state::PixcelShaderResource);
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

		vertex_buffer_resource vertexBuffer{};
		vertexBuffer.initialize(&device, sizeof(vertex), sizeof(vertex[0]));
		map(&vertexBuffer, vertex);

		index_buffer_resource indexBuffer{};
		indexBuffer.initialize(&device, sizeof(index), { component_type::UINT,32,1 });
		map(&indexBuffer, index);

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(&device, 1);
		descriptorHeap.push_back_texture2D_SRV(&device, &float4ShaderResource, 1, 0, 0, 0.f);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::SRV} }, { StaticSamplerType::Standard });

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, { &vertexShader, &pixelShader },
			{ {"POSITION", component_type::FLOAT,32,3} ,{"TEXCOOD",component_type::FLOAT,32,2} },
			{ {component_type::UNSIGNED_NORMALIZE_FLOAT,8,4} }, false, false, PrimitiveTopology::Triangle
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		while (UpdateWindow()) {

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex));

			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

			command.SetGraphicsRootSignature(&rootSignature);
			command.SetDescriptorHeap(&descriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, descriptorHeap.get_GPU_handle());
			command.SetVertexBuffer(&vertexBuffer);
			command.SetIndexBuffer(&indexBuffer);
			command.DrawIndexedInstanced(6);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::Common);

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