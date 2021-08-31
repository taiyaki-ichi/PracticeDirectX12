#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"DescriptorHeap.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/map.hpp"

#include<array>

//ƒ|ƒŠƒSƒ“‚Ì•`ŽÊ‚Ì•`ŽÊ‚ð‚·‚é
namespace test001
{

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

		using VertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>>;

		std::array<VertexLayout::type, 3> vertex{
			{{-0.8f,-0.8f,0.f},{-0.8f,0.8f,0.f},{0.8f,-0.8f,0.f}}
		};

		vertex_buffer_resource<format<component_type::FLOAT, 32, 3>> vertexBuffer{};
		vertexBuffer.initialize(&device, vertex.size());
		map(&vertexBuffer, vertex);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, {}, {});

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader001.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader001.hlsl", "main", "ps_5_0");

		PipelineState<VertexLayout,SwapChain<2>::render_target_format> pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, { &vertexShader, &pixelShader },
			{ "POSITION" }, false, false, PrimitiveTopology::Triangle
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		std::size_t cnt = 0;
		while (UpdateWindow()) {

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.0f });

			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex));

			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.SetGraphicsRootSignature(&rootSignature);

			command.SetVertexBuffer(&vertexBuffer);
			command.DrawInstanced(3);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::Common);

			command.Close();
			command.Execute();

			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());

			map(&vertexBuffer, vertex);
		};


		command.WaitAll(&device);
		return 0;
	}
}