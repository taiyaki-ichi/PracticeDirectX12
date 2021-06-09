#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"

#include<array>

namespace test001
{
	//ŽOŠpŒ`‚Ìƒ|ƒŠƒSƒ“‚Ì•`ŽÊ
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

		std::array<std::array<float, 3>, 3> vertex{
			{{-1.f,-1.f,0.f},{-1.f,1.f,0.f},{1.f,-1.f,0.f}}
		};

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(float) * 3 * 3, sizeof(float) * 3);
		vertexBufferResource.Map(vertex);

		RootSignature rootSignature{};
		rootSignature.Initialize<void, void>(&device);

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, &vertexShader, &pixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3} }, { RenderTargetFormat::R8G8B8A8 }, false);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };


		while (UpdateWindow()) {

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
			doubleBuffer.ClearBackBuffer(&commandList);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle());

			pipelineState.PrepareForDrawing(&commandList);

			commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
			commandList.Get()->DrawInstanced(3, 1, 0, 0);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}
}