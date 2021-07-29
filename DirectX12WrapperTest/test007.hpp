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
#include"Resource/ShaderResource.hpp"

#include<vector>
#include<cmath>


#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include<stb_image.h>

#include<DirectXMath.h>

namespace test007
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


	struct ComputeData {
		XMFLOAT2 ballPos{};
		float radius{};
	};


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
		vs.Intialize(L"Shader/Ground2/VertexShader.hlsl", "main", "vs_5_0");

		Shader ps{};
		ps.Intialize(L"Shader/Ground2/PixelShader.hlsl", "main", "ps_5_0");

		Shader hs{};
		hs.Intialize(L"Shader/Ground2/HullShader.hlsl", "main", "hs_5_0");

		Shader ds{};
		ds.Intialize(L"Shader/Ground2/DomainShader.hlsl", "main", "ds_5_0");

		RootSignature rootSignature{};
		rootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::SRV,DescriptorRangeType::SRV } },
			{ StaticSamplerType::Standard }
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



		constexpr std::size_t MAP_RESOURCE_EDGE_SIZE = 512;
		constexpr float MAP_CENTER = MAP_RESOURCE_EDGE_SIZE / 2.f;

		FloatShaderResource heightMapResource{};
		heightMapResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);

		Float4ShaderResource normalMapResource{};
		normalMapResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);


		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 3);
		descriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);
		descriptorHeap.PushBackView(&device, &heightMapResource);
		descriptorHeap.PushBackView(&device, &normalMapResource);


		auto [vertexList, indexList] = GetGroundPatch();

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(Vertex) * vertexList.size(), sizeof(Vertex));
		vertexBufferResource.Map(vertexList);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(std::uint16_t) * indexList.size());
		indexBufferResource.Map(indexList);


		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeHeightDescriptorHeap{};
		computeHeightDescriptorHeap.Initialize(&device, 2);

		ConstantBufferResource computeDataConstantBuffer{};
		computeDataConstantBuffer.Initialize(&device, sizeof(ComputeData));
		
		computeHeightDescriptorHeap.PushBackView(&device, &computeDataConstantBuffer);
		computeHeightDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &heightMapResource);

		Shader cs{};
		cs.Intialize(L"Shader/ComputeShader002.hlsl", "main", "cs_5_0");

		RootSignature computeHeightRootSignature{};
		computeHeightRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::UAV} },
			{}
		);

		PipelineState computeHeightPipelineState{};
		computeHeightPipelineState.Initialize(&device, &computeHeightRootSignature, &cs);



		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeNormalDescriptorHeap{};
		computeNormalDescriptorHeap.Initialize(&device, 2);

		computeNormalDescriptorHeap.PushBackView(&device, &heightMapResource);
		computeNormalDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &normalMapResource);

		Shader cs2{};
		cs2.Intialize(L"Shader/ComputeShader003.hlsl", "main", "cs_5_0");

		RootSignature computeNormalRootSignature{};
		computeNormalRootSignature.Initialize(&device,
			{ {DescriptorRangeType::SRV,DescriptorRangeType::UAV} },
			{}
		);

		PipelineState computeNormalPipelineState{};
		computeNormalPipelineState.Initialize(&device, &computeNormalRootSignature, &cs2);





		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };


		XMFLOAT3 eye{ 50, 20.f, 0.f };
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

			//
			//HeightMap‚ÌŒvŽZ
			//

			computeDataConstantBuffer.Map(ComputeData{ XMFLOAT2{100.f * std::sin(cnt / 50.f) + MAP_CENTER,100.f * std::cos(cnt / 50.f) + MAP_CENTER},10.f });

			commandList.Barrior(&heightMapResource, ResourceState::UnorderedAccessResource);
			commandList.SetComputeRootSignature(&computeHeightRootSignature);
			commandList.SetDescriptorHeap(&computeHeightDescriptorHeap);
			commandList.SetComputeRootDescriptorTable(0, computeHeightDescriptorHeap.GetGPUHandle());
			commandList.SetPipelineState(&computeHeightPipelineState);
			commandList.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			commandList.Barrior(&heightMapResource, ResourceState::PixcelShaderResource);


			//
			//NormalMap‚ÌŒvŽZ
			//

			commandList.Barrior(&normalMapResource, ResourceState::UnorderedAccessResource);
			commandList.SetComputeRootSignature(&computeNormalRootSignature);
			commandList.SetDescriptorHeap(&computeNormalDescriptorHeap);
			commandList.SetComputeRootDescriptorTable(0, computeNormalDescriptorHeap.GetGPUHandle());
			commandList.SetPipelineState(&computeNormalPipelineState);
			commandList.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			commandList.Barrior(&normalMapResource, ResourceState::PixcelShaderResource);


			cnt++;


			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);

			commandList.ClearBackBuffer(&doubleBuffer);
			commandList.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(), depthStencilDescriptorHeap.GetCPUHandle());
			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);
			commandList.SetPipelineState(&pipelineState);
			commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			commandList.SetGraphicsRootSignature(&rootSignature);
			commandList.SetDescriptorHeap(&descriptorHeap);
			commandList.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			commandList.SetVertexBuffer(&vertexBufferResource);
			commandList.SetIndexBuffer(&indexBufferResource);
			commandList.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
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