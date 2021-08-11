#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
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

		CommandList commandList{};
		commandList.Initialize(&device);

		DoubleBuffer doubleBuffer{};
		auto [factry, swapChain] = commandList.CreateFactryAndSwapChain(hwnd);
		doubleBuffer.Initialize(&device, factry, swapChain);

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


		while (UpdateWindow()) {

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);
			commandList.ClearBackBuffer(&doubleBuffer);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle());

			commandList.SetPipelineState(&pipelineState);
			commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			commandList.SetGraphicsRootSignature(&rootSignature);

			commandList.SetVertexBuffer(&vertexBufferResource);
			commandList.DrawInstanced(3);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}
}