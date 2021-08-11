#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"

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

		DescriptorHeap<DescriptorHeapTypeTag::RTV> rtvDescriptorHeap{};
		rtvDescriptorHeap.Initialize(&device, 2);
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(0));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(1));

		std::array<std::array<float, 3>, 3> vertex{
			{{-0.8f,-0.8f,0.f},{-0.8f,0.8f,0.f},{0.8f,-0.8f,0.f}}
		};

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(vertex), sizeof(decltype(vertex)::value_type));
		vertexBufferResource.Map(vertex);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, {}, {});

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader001.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader001.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, { &vertexShader, &pixelShader },
			{ {"POSITION", {Type::Float,3}} }, { {Type::UnsignedNormalizedInt8,4} }, false, false, PrimitiveTopology::Triangle
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		std::size_t cnt = 0;
		while (UpdateWindow()) {

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5f,0.5f,0.5f,1.0f });

			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex));

			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			command.SetGraphicsRootSignature(&rootSignature);

			command.SetVertexBuffer(&vertexBufferResource);
			command.DrawInstanced(3);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::Common);

			command.Close();
			command.Execute();

			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());
		};


		command.WaitAll(&device);
		return 0;
	}
}