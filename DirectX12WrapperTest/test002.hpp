#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/IndexBufferResource.hpp"
#include"Resource/TextureResource.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

#include<array>

namespace test002
{
	struct Vertex {
		float x, y, z;
		float u, v;
	};

	//テクスチャの表示
	inline int main()
	{
		using namespace DX12;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		CommandList commandList{};
		commandList.Initialize(&device);

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

		int x, y, n;
		std::uint8_t* data = stbi_load("Assets/icon.png", &x, &y, &n, 0);

		TextureResource textureResource{};
		textureResource.Initialize(&device, &commandList, data, x, y, x * n);

		using MyDescriptorTable = DescriptorTableArrayTag<
			DescriptorTableTag<DescriptorRangeTag::SRV>
		>;

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 1);

		descriptorHeap.PushBackView(&device, &textureResource);
		
		RootSignature rootSignature{};
		rootSignature.Initialize<MyDescriptorTable, StaticSamplersTag<StaticSamplerTag::Standard>>(&device);

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, &vertexShader, &pixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3} ,{"TEXCOOD",VertexLayoutFormat::Float2} },
			{ RenderTargetFormat::R8G8B8A8 }, false);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };


		while (UpdateWindow()) {

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
			doubleBuffer.ClearBackBuffer(&commandList);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle());

			pipelineState.PrepareForDrawing(&commandList);

			commandList.Get()->SetDescriptorHeaps(1, &descriptorHeap.Get());
			commandList.Get()->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
			commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());
			commandList.Get()->DrawIndexedInstanced(6, 1, 0, 0, 0);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}
}