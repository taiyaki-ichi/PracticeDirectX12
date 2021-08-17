#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/VertexBuffer.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/ConstantBuffer.hpp"
#include"Resource/DepthBuffer.hpp"
#include"Resource/ShaderResource.hpp"

#include<array>

#include<DirectXMath.h>

#include"OffLoader.hpp"
#include"utility.hpp"

//ƒ|ƒŠƒSƒ“‚Ì•`ŽÊ‚Ì•`ŽÊ‚ð‚·‚é
namespace test008
{
	using namespace DirectX;
	using namespace DX12;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct SceneData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 eye;
		XMFLOAT4 lightDir;
		XMMATRIX lightDepthViewProj;
	};

	struct GroundData
	{
		XMMATRIX world;
	};

	struct BunnyVertex {
		std::array<float, 3> pos;
		std::array<float, 3> normal;
	};

	constexpr std::uint32_t BUNNY_NUM = 3;

	struct BunnyData {
		std::array<XMMATRIX, BUNNY_NUM> world;
	};

	constexpr std::uint32_t SHADOW_MAP_EDGE = 1024;

	inline int main()
	{
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

		DepthBuffer depthBuffer{};
		depthBuffer.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> dsvDescriptorHeap{};
		dsvDescriptorHeap.Initialize(&device, 1);
		dsvDescriptorHeap.PushBackView(&device, &depthBuffer);


		//‹¤—L‚·‚éSceneData‚ÌConstantBuffer
		ConstantBuffer sceneDataConstantBuffer{};
		sceneDataConstantBuffer.Initialize(&device, sizeof(SceneData));



		//
		//ShadowMap
		//

		DepthBuffer shadowMap{};
		shadowMap.Initialize(&device, SHADOW_MAP_EDGE, SHADOW_MAP_EDGE);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> shadowMapDescriptorHeap{};
		shadowMapDescriptorHeap.Initialize(&device, 1);
		shadowMapDescriptorHeap.PushBackView(&device, &shadowMap);


		//
		//Ground
		//

		VertexBuffer groundVertexBuffer{};
		{
			std::array<std::array<float, 3>, 4> vertex{ {
				{-1.f,0.f,-1.f},
				{-1.f,0.f,1.f},
				{1.f,0.f,-1.f},
				{1.f,0.f,1.f}
				} };
			groundVertexBuffer.Initialize(&device, vertex.size(), sizeof(decltype(vertex)::value_type));
			groundVertexBuffer.Map(vertex);
		}

		IndexBuffer groundIndexBuffer{};
		std::uint32_t groundIndexNum{};
		{
			std::array<std::uint32_t, 6> index{
				0,1,2,2,1,3
			};
			groundIndexBuffer.Initialize(&device, index.size());
			groundIndexBuffer.Map(index);
			groundIndexNum = index.size();
		}

		ConstantBuffer groundDataConstantBuffer{};
		groundDataConstantBuffer.Initialize(&device, sizeof(GroundData));

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> groundDescriptorHeap{};
		groundDescriptorHeap.Initialize(&device, 3);
		groundDescriptorHeap.PushBackView(&device, &sceneDataConstantBuffer);
		groundDescriptorHeap.PushBackView(&device, &groundDataConstantBuffer);
		groundDescriptorHeap.PushBackView(&device, &shadowMap);

		RootSignature groundRootSignature{};
		groundRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::SadowMapping,StaticSamplerType::Standard }
		);

		Shader groundVS{};
		groundVS.Intialize(L"Shader/Ground3/VertexShader.hlsl", "main", "vs_5_1");

		Shader groundPS{};
		groundPS.Intialize(L"Shader/Ground3/PixelShader.hlsl", "main", "ps_5_1");

		Shader groundShadowMapVS{};
		groundShadowMapVS.Intialize(L"Shader/Ground3/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		PipelineState groundPipelineState{};
		groundPipelineState.Initialize(&device, &groundRootSignature, { &groundVS,&groundPS },
			{ {"POSITION",{Type::Float32,3}} }, { {Type::UnsignedNormalizedFloat8,4 } },
			true, false, PrimitiveTopology::Triangle
		);

		PipelineState groundShadowMapPipelineState{};
		groundShadowMapPipelineState.Initialize(&device, &groundRootSignature, { &groundShadowMapVS },
			{ {"POSITION",{Type::Float32,3}} }, {},
			true, false, PrimitiveTopology::Triangle
		);



		//
		//Bunny
		//

		VertexBuffer bunnyVertexBuffer{};
		IndexBuffer bunnyIndexBuffer{};
		std::uint32_t bunnyIndexNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bun_zipper.off");
			//auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);

			std::vector<BunnyVertex> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			bunnyIndexNum = faceList.size() * 3;

			bunnyVertexBuffer.Initialize(&device, posNormalList.size(), sizeof(BunnyVertex));
			bunnyVertexBuffer.Map(std::move(posNormalList));

			bunnyIndexBuffer.Initialize(&device, faceList.size() * 3);
			bunnyIndexBuffer.Map(std::move(faceList));
		}

		ConstantBuffer bunnyDataConstantBuffer{};
		bunnyDataConstantBuffer.Initialize(&device, sizeof(BunnyData));

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> bunnyDescriptorHeap{};
		bunnyDescriptorHeap.Initialize(&device, 3);
		bunnyDescriptorHeap.PushBackView(&device, &sceneDataConstantBuffer);
		bunnyDescriptorHeap.PushBackView(&device, &bunnyDataConstantBuffer);
		bunnyDescriptorHeap.PushBackView(&device, &shadowMap);

		RootSignature bunnyRootSignature{};
		bunnyRootSignature.Initialize(&device, 
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::SadowMapping }
		);

		Shader bunnyVS{};
		bunnyVS.Intialize(L"Shader/Bunny/VertexShader.hlsl", "main", "vs_5_1");

		Shader bunnyPS{};
		bunnyPS.Intialize(L"Shader/Bunny/PixelShader.hlsl", "main", "ps_5_1");

		Shader bunnyShadowMapVS{};
		bunnyShadowMapVS.Intialize(L"Shader/Bunny/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		PipelineState bunnyPipelineState{};
		bunnyPipelineState.Initialize(&device, &bunnyRootSignature, { &bunnyVS,&bunnyPS },
			{ {"POSITION",{Type::Float32,3}},{"NORMAL",{Type::Float32,3}} }, { {Type::UnsignedNormalizedFloat8,4} },
			true, false, PrimitiveTopology::Triangle
		);

		PipelineState bunnyShadowMapPipelineState{};
		bunnyShadowMapPipelineState.Initialize(&device, &bunnyRootSignature, { &bunnyShadowMapVS },
			{ {"POSITION",{Type::Float32,3}},{"NORMAL",{Type::Float32,3}} }, {},
			true, false, PrimitiveTopology::Triangle
		);





		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT shadowMapViewport{ 0,0, static_cast<float>(SHADOW_MAP_EDGE),static_cast<float>(SHADOW_MAP_EDGE),0.f,1.f };
		D3D12_RECT shadowMapScissorRect{ 0,0,static_cast<LONG>(SHADOW_MAP_EDGE),static_cast<LONG>(SHADOW_MAP_EDGE) };

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
		XMFLOAT3 lightDir{ 0.1f,1.f,1.f };

		auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&lightDir))
			* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

		XMMATRIX shadowMapViewProj = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(100, 100, -10.f, 100.f);

		sceneDataConstantBuffer.Map(SceneData{ view,proj,{eye.x,eye.y,eye.z,0.f}, {lightDir.x,lightDir.y,lightDir.z,0.f},shadowMapViewProj });

		groundDataConstantBuffer.Map(GroundData{ XMMatrixScaling(100.f,100.f,100.f) });

		BunnyData bunnyData{};
		//for (std::size_t i = 0; i < BUNNY_NUM; i++)
			//bunnyData.world[i] = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixTranslation(30.f - 30.f * i, 5.f, 20.f - i * 10.f);
		//bunnyDataConstantBuffer.Map(bunnyData);


		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			//
			//update
			//
			for (std::size_t i = 0; i < BUNNY_NUM; i++)
				bunnyData.world[i] = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixRotationY(cnt / 60.f) * XMMatrixTranslation(30.f , 5.f, 20.f - i * 20.f);
			bunnyDataConstantBuffer.Map(bunnyData);

			cnt++;



			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();

			command.Reset(backBufferIndex);


			//
			//ShadowMap
			//

			command.Barrior(&shadowMap, ResourceState::DepthWrite);

			command.ClearDepthView(shadowMapDescriptorHeap.GetCPUHandle(), 1.f);
			command.SetRenderTarget(std::nullopt, shadowMapDescriptorHeap.GetCPUHandle());
			command.SetViewport(shadowMapViewport);
			command.SetScissorRect(shadowMapScissorRect);

			command.SetDescriptorHeap(&bunnyDescriptorHeap);
			command.SetGraphicsRootSignature(&bunnyRootSignature);
			command.SetGraphicsRootDescriptorTable(0, bunnyDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&bunnyShadowMapPipelineState);
			command.SetVertexBuffer(&bunnyVertexBuffer);
			command.SetIndexBuffer(&bunnyIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(bunnyIndexNum, BUNNY_NUM);

			command.SetDescriptorHeap(&groundDescriptorHeap);
			command.SetGraphicsRootSignature(&groundRootSignature);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&groundShadowMapPipelineState);
			command.SetVertexBuffer(&groundVertexBuffer);
			command.SetIndexBuffer(&groundIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(groundIndexNum);

			command.Barrior(&shadowMap, ResourceState::PixcelShaderResource);



			//
			//BackBuffer
			//

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);

			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.ClearDepthView(dsvDescriptorHeap.GetCPUHandle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), dsvDescriptorHeap.GetCPUHandle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.SetDescriptorHeap(&bunnyDescriptorHeap);
			command.SetGraphicsRootSignature(&bunnyRootSignature);
			command.SetGraphicsRootDescriptorTable(0, bunnyDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&bunnyPipelineState);
			command.SetVertexBuffer(&bunnyVertexBuffer);
			command.SetIndexBuffer(&bunnyIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(bunnyIndexNum, BUNNY_NUM);

			command.SetDescriptorHeap(&groundDescriptorHeap);
			command.SetGraphicsRootSignature(&groundRootSignature);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&groundPipelineState);
			command.SetVertexBuffer(&groundVertexBuffer);
			command.SetIndexBuffer(&groundIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(groundIndexNum);


			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::Common);

			command.Close();
			command.Execute();

			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());
		}

		command.WaitAll(&device);

		return 0;
	}
}