#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"Resource/VertexBuffer.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/IndexBuffer.hpp"
#include"Resource/DepthBuffer.hpp"
#include"Resource/ConstantBuffer.hpp"

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

		Command command{};
		command.Initialize(&device);

		auto swapChain = command.CreateSwapChain(&device, hwnd);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> rtvDescriptorHeap{};
		rtvDescriptorHeap.Initialize(&device, 2);
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(0));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(1));

		auto [vertex, face] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bunny.off");
		
		VertexBuffer vertexBuffer{};
		vertexBuffer.Initialize(&device, vertex.size(), sizeof(decltype(vertex)::value_type));
		vertexBuffer.Map(vertex);

		IndexBuffer indexBuffer{};
		indexBuffer.Initialize(&device, face.size() * 3);
		indexBuffer.Map(face);

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
			{ {"POSITION", {Type::Float,3}} }, { {Type::UnsignedNormalizedFloat,4} }, true, false, PrimitiveTopology::Triangle
		);

		PipelineState drawNormalPipelineState{};
		drawNormalPipelineState.Initialize(&device, &rootSignature, { &vertexShader, &drawNormalPixelShader,&drawNormalGeometryShader },
			{ {"POSITION", {Type::Float,3}} }, { {Type::UnsignedNormalizedFloat,4} }, true, false, PrimitiveTopology::Triangle
		);


		DepthBuffer depthBuffer{};
		depthBuffer.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthBuffer);

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

		ConstantBuffer sceneDataConstantBuffer{};
		sceneDataConstantBuffer.Initialize(&device, sizeof(SceneData));
		sceneDataConstantBuffer.Map(SceneData{ view,proj });

		//SceneDataをシェーダに渡す用
		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 1);
		descriptorHeap.PushBackView(&device, &sceneDataConstantBuffer);

		std::size_t cnt = 0;
		while (UpdateWindow()) {

			eye.x = len * std::cos(cnt / 60.0);
			eye.z = len * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			sceneDataConstantBuffer.Map(SceneData{ view,proj });
			cnt++;

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5,0.5,0.5,1.0 });

			command.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);

			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex),depthStencilDescriptorHeap.GetCPUHandle());

			//面の描写
			{
				command.SetPipelineState(&drawFacePipelineState);
				command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
				command.SetGraphicsRootSignature(&rootSignature);

				command.SetVertexBuffer(&vertexBuffer);
				command.SetIndexBuffer(&indexBuffer);
				command.SetDescriptorHeap(&descriptorHeap);
				command.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());

				command.DrawIndexedInstanced(face.size() * 3);
			}

			//法線の描写
			{
				command.SetPipelineState(&drawNormalPipelineState);
				command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
				command.SetGraphicsRootSignature(&rootSignature);

				command.SetVertexBuffer(&vertexBuffer);
				command.SetIndexBuffer(&indexBuffer);
				command.SetDescriptorHeap(&descriptorHeap);
				command.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());

				command.DrawIndexedInstanced(face.size() * 3);
			}

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