#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/IndexBufferResource.hpp"
#include"Resource/DepthBufferResource.hpp"
#include"Resource/ConstantBufferResource.hpp"

#include"OffLoader.hpp"

#include<array>
#include<cmath>
#include<DirectXMath.h>

//ジオメトリシェーダを使用し法線などを描画
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

		auto [vertex, face] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("../../Assets/bunny.off");
		
		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(decltype(vertex)::value_type) * vertex.size(), sizeof(decltype(vertex)::value_type));
		vertexBufferResource.Map(vertex);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(decltype(face)::value_type) * face.size());
		indexBufferResource.Map(face);

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::CBV} }, {});

		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader003.hlsl", "main", "vs_5_0");

		Shader drawFacePixelShader{};
		drawFacePixelShader.Intialize(L"Shader/PixelShader003_DrawFace.hlsl", "main", "ps_5_0");

		Shader drawFaceGeometryShader{};
		drawFaceGeometryShader.Intialize(L"Shader/GeometryShader003_DrawFace.hlsl", "main", "gs_5_0");

		Shader drawNormalPixelShader{};
		drawNormalPixelShader.Intialize(L"Shader/PixelShader003_DrawNormal.hlsl", "main", "ps_5_0");

		Shader drawNormalGeometryShader{};
		drawNormalGeometryShader.Intialize(L"Shader/GeometryShader003_DrawNormal.hlsl", "main", "gs_5_0");

		PipelineState drawFacePipelineState{};
		drawFacePipelineState.Initialize(&device, &rootSignature, { &vertexShader, &drawFacePixelShader,&drawFaceGeometryShader },
			{ {"POSITION", {Type::Float,3}} }, { {Type::UnsignedNormalizedInt8,4} }, true, false, PrimitiveTopology::Triangle
		);

		PipelineState drawNormalPipelineState{};
		drawNormalPipelineState.Initialize(&device, &rootSignature, { &vertexShader, &drawNormalPixelShader,&drawNormalGeometryShader },
			{ {"POSITION", {Type::Float,3}} }, { {Type::UnsignedNormalizedInt8,4} }, true, false, PrimitiveTopology::Triangle
		);


		DepthBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);

		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		float len = 0.15;
		XMFLOAT3 eye{ 0,len,len };
		XMFLOAT3 target{ 0,0.1,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.01f,
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

			eye.x = len * std::cos(cnt / 60.0);
			eye.z = len * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			sceneDataConstantBufferResource.Map(SceneData{ view,proj });
			cnt++;

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);
			commandList.ClearBackBuffer(&doubleBuffer);

			commandList.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(),depthStencilDescriptorHeap.GetCPUHandle());

			//面の描写
			{
				commandList.SetPipelineState(&drawFacePipelineState);
				commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
				commandList.SetGraphicsRootSignature(&rootSignature);

				commandList.SetVertexBuffer(&vertexBufferResource);
				commandList.SetIndexBuffer(&indexBufferResource);
				commandList.SetDescriptorHeap(&descriptorHeap);
				commandList.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());

				commandList.DrawIndexedInstanced(face.size() * 3);
			}

			//法線の描写
			{
				commandList.SetPipelineState(&drawNormalPipelineState);
				commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
				commandList.SetGraphicsRootSignature(&rootSignature);

				commandList.SetVertexBuffer(&vertexBufferResource);
				commandList.SetIndexBuffer(&indexBufferResource);
				commandList.SetDescriptorHeap(&descriptorHeap);
				commandList.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());

				commandList.DrawIndexedInstanced(face.size() * 3);
			}

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		};

		return 0;
	}
}