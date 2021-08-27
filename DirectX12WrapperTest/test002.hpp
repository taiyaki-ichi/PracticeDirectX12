#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"DescriptorHeap.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"Resource/map.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/shader_resource.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

#include<array>

//テクスチャの表示をする
namespace test002
{
	struct Vertex {
		float x, y, z;
		float u, v;
	};

	inline int main()
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

		std::array<Vertex, 4> vertex{ {
			{-0.8f,-0.8f,0.f,0.f,1.f},
			{-0.8f,0.8f,0.f,0.f,0.f},
			{0.8f,-0.8f,0.f,1.f,1.f},
			{0.8f,0.8f,0.f,1.f,0.f}
			} };

		std::array<std::uint32_t, 6> index{
			0,1,2,2,1,3
		};

		vertex_buffer_resource<format<component_type::FLOAT,32,3>,format<component_type::FLOAT,32,2>> vertexBuffer{};
		vertexBuffer.initialize(&device, vertex.size());
		map(&vertexBuffer, vertex);

		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(&device, index.size());
		map(&indexBuffer, index);

		int x, y, n;
		std::uint8_t* data = stbi_load("../../Assets/icon.png", &x, &y, &n, 0);

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> textureResource{};
		{
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
		}

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(&device, 1);

		descriptorHeap.push_back_texture2D_SRV(&device, &textureResource, 1, 0, 0, 0.f);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::SRV} }, { StaticSamplerType::Standard });

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, { &vertexShader, &pixelShader },
			{ {"POSITION", component_type::FLOAT,32,3} ,{"TEXCOOD",component_type::FLOAT ,32,2} },
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
			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });

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
		};

		stbi_image_free(data);
		command.WaitAll(&device);

		return 0;
	}
}