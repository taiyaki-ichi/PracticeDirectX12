#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/DepthStencilBufferResource.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/ConstantBufferResource.hpp"
#include"Resource/TextureResource.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"Resource/IndexBufferResource.hpp"

#include<vector>

#include<DirectXMath.h>

namespace test006
{
	using namespace DX12;
	using namespace DirectX;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct SceneData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX world;
		XMFLOAT3 eye;
		XMFLOAT4 tessRange;
	};

	struct Vertex {
		float x, y, z;
		float uvX, uvY;
	};

	inline std::pair<std::vector<Vertex>, std::vector<std::uint16_t>> GetGroundPatch()
	{
		std::vector<Vertex> vertexList{};
		std::vector<std::uint16_t> indexList{};

		constexpr float EDGE = 200.f;
		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = EDGE * x / DIVIDE;
				auto posZ = EDGE * z / DIVIDE;
				vertexList.push_back(Vertex{ posX,0.f,posZ,posX / EDGE,posZ / EDGE });
			}
		}

		indexList.reserve(DIVIDE * DIVIDE);
		for (std::size_t z = 0; z < DIVIDE; z++) {
			for (std::size_t x = 0; x < DIVIDE; x++) {
				auto v0 = x;
				auto v1 = x + 1;

				v0 += ROWS * z;
				v1 += ROWS * z;

				indexList.push_back(v0 + ROWS);
				indexList.push_back(v1 + ROWS);
				indexList.push_back(v0);
				indexList.push_back(v1);
			}
		}

		for (auto& v : vertexList) {
			v.x -= EDGE / 2.f;
			v.z -= EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
	}


	inline int main()
	{
		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		CommandList commandList{};
		commandList.Initialize(&device);

		DoubleBuffer doubleBuffer{};
		auto [factry, swapChain] = commandList.CreateFactryAndSwapChain(hwnd);
		doubleBuffer.Initialize(&device, factry, swapChain);

		Shader vs{};
		vs.Intialize(L"Shader/WaveGround/VertexShader.hlsl", "main", "vs_5_0");

		Shader ps{};
		ps.Intialize(L"Shader/WaveGround/PixelShader.hlsl", "main", "ps_5_0");

		Shader hs{};
		hs.Intialize(L"Shader/WaveGround/HullShader.hlsl", "main", "hs_5_0");

		Shader ds{};
		ds.Intialize(L"Shader/WaveGround/DomainShader.hlsl", "main", "ds_5_0");

		RootSignature rootSignature{};
		rootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV} },
			{}
		);

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, &vs, &ps,
			{ {"POSITION",VertexLayoutFormat::Float3},{"TEXCOOD",VertexLayoutFormat::Float2} },
			{ Format::R8G8B8A8 }, true, nullptr, &hs, &ds);


		DepthStencilBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);


		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 1);
		descriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);


		auto [vertexList, indexList] = GetGroundPatch();

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(Vertex) * vertexList.size(), sizeof(Vertex));
		vertexBufferResource.Map(vertexList);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(std::uint16_t) * indexList.size());
		indexBufferResource.Map(indexList);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		XMFLOAT3 eye{ 20, 10.f, 20.f };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			500.f
		);

		sceneDataConstantBufferResource.Map(SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) });

		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			//eye.x = len * std::cos(cnt / 60.0) - len / 3.f * 2.f;
			//eye.z = len * std::sin(cnt / 600.0);
			//XMFLOAT3 t{ eye.x + 10.f,-1.f,0 };
			//auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&t), XMLoadFloat3(&up));
			sceneDataConstantBufferResource.Map(SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,50.f,4.f,0.f) });
			//cnt++;


			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);

			commandList.ClearBackBuffer(&doubleBuffer);
			commandList.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(), depthStencilDescriptorHeap.GetCPUHandle());
			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);
			commandList.SetPipelineState(&pipelineState);
			commandList.SetDescriptorHeap(&descriptorHeap);
			commandList.SetRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			commandList.SetVertexBuffer(&vertexBufferResource);
			commandList.SetIndexBuffer(&indexBufferResource);
			commandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
			commandList.DrawIndexedInstanced(indexList.size());

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		}

		return 0;
	}



}