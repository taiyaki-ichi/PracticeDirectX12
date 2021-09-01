#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"DescriptorHeap.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/constant_buffer_resource.hpp"
#include"Resource/shader_resource.hpp"
#include"Resource/map.hpp"

#include<array>

#include<DirectXMath.h>

#include"OffLoader.hpp"
#include"utility.hpp"

//�|���S���̕`�ʂ̕`�ʂ�����
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

	using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

	using BunnyVertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3>>;

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
		command.Initialize(device);

		auto swapChain = command.CreateSwapChain<FrameBufferFormat>(hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.GetFrameBuffer(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.GetFrameBuffer(1), 0, 0);

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { {1.f} });

		descriptor_heap_DSV dsvDescriptorHeap{};
		dsvDescriptorHeap.initialize(device, 1);
		dsvDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);


		//���L����SceneData��ConstantBuffer
		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);



		//
		//ShadowMap
		//

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> shadowMap{};
		shadowMap.initialize(device, SHADOW_MAP_EDGE, SHADOW_MAP_EDGE, 1, 1, { {1.f} });

		descriptor_heap_DSV shadowMapDescriptorHeap{};
		shadowMapDescriptorHeap.initialize(device, 1);
		shadowMapDescriptorHeap.push_back_texture2D_DSV(device, shadowMap, 0);


		//
		//Ground
		//

		vertex_buffer_resource<format<component_type::FLOAT, 32, 3>> groundVertexBuffer{};
		{
			std::array<std::array<float, 3>, 4> vertex{ {
				{-1.f,0.f,-1.f},
				{-1.f,0.f,1.f},
				{1.f,0.f,-1.f},
				{1.f,0.f,1.f}
				} };
			groundVertexBuffer.initialize(device, vertex.size());
			map(&groundVertexBuffer, std::move(vertex));
		}

		index_buffer_resource<format<component_type::UINT,32,1>> groundIndexBuffer{};
		std::uint32_t groundIndexNum{};
		{
			std::array<std::uint32_t, 6> index{
				0,1,2,2,1,3
			};
			groundIndexBuffer.initialize(device, index.size());
			map(&groundIndexBuffer, std::move(index));
			groundIndexNum = index.size();
		}

		constant_buffer_resource<GroundData> groundDataConstantBuffer{};
		groundDataConstantBuffer.initialize(device);

		descriptor_heap_CBV_SRV_UAV groundDescriptorHeap{};
		groundDescriptorHeap.initialize(device, 3);
		groundDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		groundDescriptorHeap.push_back_CBV(device, groundDataConstantBuffer);
		groundDescriptorHeap.push_back_texture2D_SRV(device, shadowMap, 1, 0, 0, 0.f);

		RootSignature groundRootSignature{};
		groundRootSignature.Initialize(device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::SadowMapping,StaticSamplerType::Standard }
		);

		Shader groundVS{};
		groundVS.Intialize(L"Shader/Ground3/VertexShader.hlsl", "main", "vs_5_1");

		Shader groundPS{};
		groundPS.Intialize(L"Shader/Ground3/PixelShader.hlsl", "main", "ps_5_1");

		Shader groundShadowMapVS{};
		groundShadowMapVS.Intialize(L"Shader/Ground3/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		graphics_pipeline_state<vertex_layout<format<component_type::FLOAT,32,3>>,render_target_formats<FrameBufferFormat>> groundPipelineState{};
		groundPipelineState.Initialize(device, groundRootSignature, { &groundVS,&groundPS },
			{ "POSITION" }, true, false, PrimitiveTopology::Triangle
		);

		graphics_pipeline_state<vertex_layout<format<component_type::FLOAT, 32, 3>>, render_target_formats<>> groundShadowMapPipelineState{};
		groundShadowMapPipelineState.Initialize(device, groundRootSignature, { &groundShadowMapVS },
			{ "POSITION" }, true, false, PrimitiveTopology::Triangle
		);



		//
		//Bunny
		//

		vertex_buffer_resource<format<component_type::FLOAT,32,3>, format<component_type::FLOAT, 32, 3>> bunnyVertexBuffer{};
		index_buffer_resource<format<component_type::UINT,32,1>> bunnyIndexBuffer{};
		std::uint32_t bunnyIndexNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bun_zipper.off");
			//auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);

			std::vector<BunnyVertexLayout::struct_type> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			bunnyIndexNum = faceList.size() * 3;

			bunnyVertexBuffer.initialize(device, posNormalList.size());
			map(&bunnyVertexBuffer, posNormalList.begin(), posNormalList.end());

			bunnyIndexBuffer.initialize(device, faceList.size() * 3);
			map(&bunnyIndexBuffer, faceList.begin(), faceList.end());
		}

		constant_buffer_resource<BunnyData> bunnyDataConstantBuffer{};
		bunnyDataConstantBuffer.initialize(device);

		descriptor_heap_CBV_SRV_UAV bunnyDescriptorHeap{};
		bunnyDescriptorHeap.initialize(device, 3);
		bunnyDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		bunnyDescriptorHeap.push_back_CBV(device, bunnyDataConstantBuffer);
		bunnyDescriptorHeap.push_back_texture2D_SRV(device, shadowMap, 1, 0, 0, 0.f);

		RootSignature bunnyRootSignature{};
		bunnyRootSignature.Initialize(device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::SadowMapping,StaticSamplerType::Standard }
		);

		Shader bunnyVS{};
		bunnyVS.Intialize(L"Shader/Bunny/VertexShader.hlsl", "main", "vs_5_1");

		Shader bunnyPS{};
		bunnyPS.Intialize(L"Shader/Bunny/PixelShader.hlsl", "main", "ps_5_1");

		Shader bunnyShadowMapVS{};
		bunnyShadowMapVS.Intialize(L"Shader/Bunny/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		graphics_pipeline_state<BunnyVertexLayout,render_target_formats<FrameBufferFormat>> bunnyPipelineState{};
		bunnyPipelineState.Initialize(device, bunnyRootSignature, { &bunnyVS,&bunnyPS },
			{ "POSITION","NORMAL" },
			true, false, PrimitiveTopology::Triangle
		);

		graphics_pipeline_state<BunnyVertexLayout,render_target_formats<>> bunnyShadowMapPipelineState{};
		bunnyShadowMapPipelineState.Initialize(device, bunnyRootSignature, { &bunnyShadowMapVS },
			{ "POSITION","NORMAL"},true, false, PrimitiveTopology::Triangle
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
		XMFLOAT3 lightDir{ 0.f,1.f,1.f };

		auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&lightDir))
			* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

		XMMATRIX shadowMapViewProj = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(100, 100, -100.f, 200.f);

		map(&sceneDataConstantBuffer, SceneData{ view,proj,{eye.x,eye.y,eye.z,0.f}, {lightDir.x,lightDir.y,lightDir.z,0.f},shadowMapViewProj });

		map(&groundDataConstantBuffer, GroundData{ XMMatrixScaling(100.f,100.f,100.f) });

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
				bunnyData.world[i] = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixRotationY(cnt / 60.f) * XMMatrixTranslation(30.f , 0.f, 40.f - i * 40.f);
			map(&bunnyDataConstantBuffer, bunnyData);

			cnt++;



			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();

			command.Reset(backBufferIndex);

		

			//
			//ShadowMap
			//

			command.Barrior(shadowMap, resource_state::DepthWrite);

			command.ClearDepthView(shadowMapDescriptorHeap.get_CPU_handle(), 1.f);
			command.SetRenderTarget(std::nullopt, shadowMapDescriptorHeap.get_CPU_handle());
			command.SetViewport(shadowMapViewport);
			command.SetScissorRect(shadowMapScissorRect);

			command.SetDescriptorHeap(bunnyDescriptorHeap);
			command.SetGraphicsRootSignature(bunnyRootSignature);
			command.SetGraphicsRootDescriptorTable(0, bunnyDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(bunnyShadowMapPipelineState);
			command.SetVertexBuffer(bunnyVertexBuffer);
			command.SetIndexBuffer(bunnyIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(bunnyIndexNum, BUNNY_NUM);

			command.SetDescriptorHeap(groundDescriptorHeap);
			command.SetGraphicsRootSignature(groundRootSignature);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(groundShadowMapPipelineState);
			command.SetVertexBuffer(groundVertexBuffer);
			command.SetIndexBuffer(groundIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(groundIndexNum);

			command.Barrior(shadowMap, resource_state::PixcelShaderResource);



			//
			//BackBuffer
			//

			command.Barrior(swapChain.GetFrameBuffer(backBufferIndex), resource_state::RenderTarget);
			command.Barrior(depthBuffer, resource_state::DepthWrite);

			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.ClearDepthView(dsvDescriptorHeap.get_CPU_handle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), dsvDescriptorHeap.get_CPU_handle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.SetDescriptorHeap(bunnyDescriptorHeap);
			command.SetGraphicsRootSignature(bunnyRootSignature);
			command.SetGraphicsRootDescriptorTable(0, bunnyDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(bunnyPipelineState);
			command.SetVertexBuffer(bunnyVertexBuffer);
			command.SetIndexBuffer(bunnyIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(bunnyIndexNum, BUNNY_NUM);

			command.SetDescriptorHeap(groundDescriptorHeap);
			command.SetGraphicsRootSignature(groundRootSignature);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(groundPipelineState);
			command.SetVertexBuffer(groundVertexBuffer);
			command.SetIndexBuffer(groundIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(groundIndexNum);


			command.Barrior(swapChain.GetFrameBuffer(backBufferIndex), resource_state::Common);

			command.Close();
			command.Execute();

			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());
		}

		command.WaitAll(device);

		return 0;
	}
}