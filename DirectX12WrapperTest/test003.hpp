#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/IndexBufferResource.hpp"
#include"Resource/DepthStencilBufferResource.hpp"
#include"Resource/ConstantBufferResource.hpp"

#include"OffLoader.hpp"

#include<array>
#include<cmath>
#include<DirectXMath.h>


namespace test003
{
	struct SceneData{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
	};

	inline int main()
	{
		using namespace DX12;
		using namespace DirectX;

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

		auto [vertex, face] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("Assets/tetra.off");

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(decltype(vertex)::value_type) * vertex.size(), sizeof(decltype(vertex)::value_type));
		vertexBufferResource.Map(vertex);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(face));
		indexBufferResource.Map(face);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::CBV} }, {});

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader003.hlsl", "main", "vs_5_0");

		Shader pixelShader{};
		pixelShader.Intialize(L"Shader/PixelShader003.hlsl", "main", "ps_5_0");

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, &vertexShader, &pixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3} }, { RenderTargetFormat::R8G8B8A8 }, true);

		DepthStencilBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);

		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		XMFLOAT3 eye{ 0,3,3 };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			1.f,
			100.f
		);

		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));
		sceneDataConstantBufferResource.Map(SceneData{ view,proj });

		//SceneDataをシェーダに渡す用
		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 1);
		descriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);

		std::size_t cnt = 0;
		while (UpdateWindow()) {

			eye.z = 5 * std::cos(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			sceneDataConstantBufferResource.Map(SceneData{ view,proj });
			cnt++;

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
			doubleBuffer.ClearBackBuffer(&commandList);

			commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(),
				D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(),depthStencilDescriptorHeap.GetCPUHandle());

			pipelineState.PrepareForDrawing(&commandList);

			commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
			commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());
			commandList.Get()->SetDescriptorHeaps(1, &descriptorHeap.Get());
			commandList.Get()->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());

			commandList.Get()->DrawIndexedInstanced(face.size() * 3, 1, 0, 0, 0);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}
}