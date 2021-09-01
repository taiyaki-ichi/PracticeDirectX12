#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"DescriptorHeap.hpp"
#include"Resource/constant_buffer_resource.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"Resource/shader_resource.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"
#include"Resource/map.hpp"

#include<vector>
#include<cmath>
#include<random>
#include<chrono>


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
		XMFLOAT3 eye;
		float offset;
		XMFLOAT3 lightDir;
	};

	struct GroundData {
		XMMATRIX world;
	};

	using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

	using VertexLayout1 = vertex_layout<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;
	using VertexLayout2 = vertex_layout<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3>>;

	struct SphereData {
		std::array<XMMATRIX, 2> world;
	};

	struct SnowData {
		XMFLOAT4 move;//ê·ÇÃà⁄ìÆó 
		XMFLOAT4 center;//ê·ÇêUÇÁÇπÇÈîÕàÕÇÃíÜêS
		float range;//ê·ÇÃîÕàÕ
		float rangeR;//rangeÇÃãtêî
		float size;//ê·ÇÃëÂÇ´Ç≥
	};

	constexpr std::size_t MAP_RESOURCE_EDGE_SIZE = 1024;

	constexpr float GROUND_EDGE = 128.f;

	constexpr std::size_t FRAME_BUFFER_NUM = 3;

	inline std::pair<std::vector<VertexLayout1::struct_type>, std::vector<std::uint32_t>> GetGroundPatch()
	{
		std::vector<VertexLayout1::struct_type> vertexList{};
		std::vector<std::uint32_t> indexList{};

		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = GROUND_EDGE * x / DIVIDE;
				auto posZ = GROUND_EDGE * z / DIVIDE;
				vertexList.push_back({ { posX,0.f,posZ},{posX / GROUND_EDGE,posZ / GROUND_EDGE} });
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
			std::get<0>(v)[0] -= GROUND_EDGE / 2.f;
			std::get<0>(v)[2] -= GROUND_EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
	}


	inline int main()
	{
		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		Command<FRAME_BUFFER_NUM> command{};
		command.Initialize(device);

		auto swapChain = command.CreateSwapChain<FrameBufferFormat>(hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, FRAME_BUFFER_NUM);
		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.GetFrameBuffer(i), 0, 0);

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { {1.f} });

		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(device, 1);
		depthStencilDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);


		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);


		Shader groundVS{};
		groundVS.Intialize(L"Shader/Ground2/VertexShader.hlsl", "main", "vs_5_1");

		Shader grooundPS{};
		grooundPS.Intialize(L"Shader/Ground2/PixelShader.hlsl", "main", "ps_5_1");

		Shader groundHS{};
		groundHS.Intialize(L"Shader/Ground2/HullShader.hlsl", "main", "hs_5_1");

		Shader groundDS{};
		groundDS.Intialize(L"Shader/Ground2/DomainShader.hlsl", "main", "ds_5_1");

		RootSignature groundRootSignature{};
		groundRootSignature.Initialize(device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV,DescriptorRangeType::SRV,
				DescriptorRangeType::SRV,DescriptorRangeType::SRV,
				DescriptorRangeType::SRV} },
			{ StaticSamplerType::Standard }
		);

		graphics_pipeline_state<VertexLayout1,render_target_formats<FrameBufferFormat>> groundPipelineState{};
		groundPipelineState.Initialize(device, groundRootSignature, { &groundVS, &grooundPS ,nullptr,&groundHS, &groundDS },
			{ "POSITION","TEXCOOD" }, true, false, PrimitiveTopology::Patch
		);


		constant_buffer_resource<GroundData> groundDataConstantBuffer{};
		groundDataConstantBuffer.initialize(device);
		map(&groundDataConstantBuffer, GroundData{ XMMatrixIdentity() });
		

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowUnorderdAccess> heightMapResource{};
		heightMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);

		shader_resource<format<component_type::UINT, 32, 1>, resource_flag::AllowUnorderdAccess> elapsedTimeMapResource{};
		elapsedTimeMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>, resource_flag::AllowUnorderdAccess> normalMapResource{};
		normalMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);


		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> groundDepthTextureResource{};
		{
			int x, y, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_DISP.png", &x, &y, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(device, TextureDataPitchAlignment(x * 4) * y);
			map(&uploadResource, data, x * 4, y, TextureDataPitchAlignment(x * 4));

			groundDepthTextureResource.initialize(device, x, y, 1, 1);

			command.Reset(0);
			command.Barrior(groundDepthTextureResource, resource_state::CopyDest);
			command.CopyTexture(device, uploadResource, groundDepthTextureResource);
			command.Barrior(groundDepthTextureResource, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> groundNormalTextureResource{};
		{
			int x, y, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_NORM.jpg", &x, &y, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(device, TextureDataPitchAlignment(x * 4) * y);
			map(&uploadResource, data, x * 4, y, TextureDataPitchAlignment(x * 4));

			groundNormalTextureResource.initialize(device, x, y, 1, 1);

			command.Reset(0);
			command.Barrior(groundNormalTextureResource, resource_state::CopyDest);
			command.CopyTexture(device, uploadResource, groundNormalTextureResource);
			command.Barrior(groundNormalTextureResource, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		descriptor_heap_CBV_SRV_UAV groundDescriptorHeap{};
		groundDescriptorHeap.initialize(device, 7);
		groundDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		groundDescriptorHeap.push_back_CBV(device, groundDataConstantBuffer);
		groundDescriptorHeap.push_back_texture2D_SRV(device, heightMapResource, 1, 0, 0, 0.f);
		groundDescriptorHeap.push_back_texture2D_SRV(device, normalMapResource, 1, 0, 0, 0.f);
		groundDescriptorHeap.push_back_texture2D_SRV(device, groundDepthTextureResource, 1, 0, 0, 0.f);
		groundDescriptorHeap.push_back_texture2D_SRV(device, groundNormalTextureResource, 1, 0, 0, 0.f);
		groundDescriptorHeap.push_back_texture2D_SRV(device, elapsedTimeMapResource, 1, 0, 0, 0.f);

		auto [vertexList, indexList] = GetGroundPatch();

		VertexLayout1::resource_type vertexBuffer{};
		vertexBuffer.initialize(device, vertexList.size());
		map(&vertexBuffer, vertexList.begin(), vertexList.end());

		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, indexList.size());
		map(&indexBuffer, indexList.begin(), indexList.end());


		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowRenderTarget> groundDepthShaderResource{};
		groundDepthShaderResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1, { {0.f} });

		descriptor_heap_CBV_SRV_UAV computeHeightDescriptorHeap{};
		computeHeightDescriptorHeap.initialize(device, 3);
		computeHeightDescriptorHeap.push_back_texture2D_UAV(device, heightMapResource, 0, 0);
		computeHeightDescriptorHeap.push_back_texture2D_UAV(device, elapsedTimeMapResource, 0, 0);
		computeHeightDescriptorHeap.push_back_texture2D_SRV(device, groundDepthShaderResource, 1, 0, 0, 0.f);

		Shader computeHeightCS{};
		computeHeightCS.Intialize(L"Shader/ComputeShader002.hlsl", "main", "cs_5_1");

		RootSignature computeHeightRootSignature{};
		computeHeightRootSignature.Initialize(device,
			{ {DescriptorRangeType::UAV,DescriptorRangeType::UAV,DescriptorRangeType::SRV} },
			{}
		);

		compute_pipeline_state computeHeightPipelineState{};
		computeHeightPipelineState.Initialize(device, computeHeightRootSignature, computeHeightCS);

		descriptor_heap_CBV_SRV_UAV computeNormalDescriptorHeap{};
		computeNormalDescriptorHeap.initialize(device, 2);
		computeNormalDescriptorHeap.push_back_texture2D_SRV(device, heightMapResource, 1, 0, 0, 0.f);
		computeNormalDescriptorHeap.push_back_texture2D_UAV(device, normalMapResource, 0, 0);

		Shader computeNormalCS{};
		computeNormalCS.Intialize(L"Shader/ComputeShader003.hlsl", "main", "cs_5_1");

		RootSignature computeNormalRootSignature{};
		computeNormalRootSignature.Initialize(device,
			{ {DescriptorRangeType::SRV,DescriptorRangeType::UAV} },
			{}
		);

		compute_pipeline_state computeNormalPipelineState{};
		computeNormalPipelineState.Initialize(device, computeNormalRootSignature, computeNormalCS);


		vertex_buffer_resource<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3 >> sphereVertexBuffer{};
		index_buffer_resource<format<component_type::UINT,32,1>> sphereIndexBuffer{};
		std::size_t sphereFaceNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);


			std::vector<VertexLayout2::struct_type> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			sphereFaceNum = faceList.size();

			sphereVertexBuffer.initialize(device, posNormalList.size());
			map(&sphereVertexBuffer, posNormalList.begin(), posNormalList.end());

			sphereIndexBuffer.initialize(device, faceList.size() * 3);
			map(&sphereIndexBuffer, faceList.begin(), faceList.end());
		}


		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> groundDepthBuffer{};
		groundDepthBuffer.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1, { {1.f} });

		descriptor_heap_DSV groundDepthStencilDescriptorHeap{};
		groundDepthStencilDescriptorHeap.initialize(device, 1);
		groundDepthStencilDescriptorHeap.push_back_texture2D_DSV(device, groundDepthBuffer, 0);

		descriptor_heap_RTV groundDepthRTVDescriptorHeap{};
		groundDepthRTVDescriptorHeap.initialize(device, 1);
		groundDepthRTVDescriptorHeap.push_back_texture2D_RTV(device, groundDepthShaderResource, 0, 0);


		Shader sphereVS{};
		sphereVS.Intialize(L"Shader/Sphere/VertexShader.hlsl", "main", "vs_5_1");

		Shader spherePS{};
		spherePS.Intialize(L"Shader/Sphere/PixelShader.hlsl", "main", "ps_5_1");

		Shader sphereDepthVS{};
		sphereDepthVS.Intialize(L"Shader/Sphere/DepthVertexShader.hlsl", "main", "vs_5_1");

		Shader sphereDepthPS{};
		sphereDepthPS.Intialize(L"Shader/Sphere/DepthPixelShader.hlsl", "main", "ps_5_1");

		constant_buffer_resource<SphereData> sphereDataConstantBuffer{};
		sphereDataConstantBuffer.initialize(device);

		constant_buffer_resource<XMMATRIX> upCameraMatrixConstantBuffer{};
		upCameraMatrixConstantBuffer.initialize(device);

		RootSignature sphereRootSignature{};
		sphereRootSignature.Initialize(device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::CBV} },
			{}
		);

		graphics_pipeline_state<VertexLayout2,render_target_formats<FrameBufferFormat>> spherePipelineState{};
		spherePipelineState.Initialize(device, sphereRootSignature, { &sphereVS, &spherePS },
			{ "POSITION","NORMAL" }, true, false, PrimitiveTopology::Triangle
		);

		graphics_pipeline_state<VertexLayout2,render_target_formats<format<component_type::FLOAT,32,1>>> sphereDepthPipelineState{};
		sphereDepthPipelineState.Initialize(device, sphereRootSignature, { &sphereDepthVS, &sphereDepthPS },
			{ "POSITION","NORMAL" }, true, false, PrimitiveTopology::Triangle
		);


		descriptor_heap_CBV_SRV_UAV sphereDescriptorHeap{};
		sphereDescriptorHeap.initialize(device, 3);
		sphereDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		sphereDescriptorHeap.push_back_CBV(device, sphereDataConstantBuffer);
		sphereDescriptorHeap.push_back_CBV(device, upCameraMatrixConstantBuffer);


		constexpr std::size_t SNOW_NUM = 8000;
		constexpr float SNOW_RANGE = 8.f;
		vertex_buffer_resource<format<component_type::FLOAT,32,3>> snowVertexBuffer{};
		{
			std::random_device seed_gen;
			std::default_random_engine engine(seed_gen());
			std::uniform_real_distribution<float> dist(-SNOW_RANGE, SNOW_RANGE);
			std::vector<std::array<float, 3>> v{};
			v.reserve(SNOW_NUM);
			for (std::size_t i = 0; i < SNOW_NUM; i++)
				v.emplace_back(std::array<float, 3>{dist(engine), dist(engine), dist(engine)});
			snowVertexBuffer.initialize(device, SNOW_NUM);
			map(&snowVertexBuffer, v.begin(), v.end());
		}

		constant_buffer_resource<SnowData> snowDataConstantBuffer{};
		snowDataConstantBuffer.initialize(device);


		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> snowTextureShader{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/snow.png", &textureWidth, &textureHeight, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(device, TextureDataPitchAlignment(textureWidth * 4)* textureHeight);
			map(&uploadResource, data, textureWidth * 4, textureHeight, TextureDataPitchAlignment(textureWidth * 4));

			snowTextureShader.initialize(device, textureWidth, textureHeight, 1, 1);

			command.Reset(0);
			command.Barrior(snowTextureShader, resource_state::CopyDest);
			command.CopyTexture(device, uploadResource, snowTextureShader);
			command.Barrior(snowTextureShader, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}


		descriptor_heap_CBV_SRV_UAV snowDescriptorHeap{};
		snowDescriptorHeap.initialize(device, 3);
		snowDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		snowDescriptorHeap.push_back_CBV(device, snowDataConstantBuffer);
		snowDescriptorHeap.push_back_texture2D_SRV(device, snowTextureShader, 1, 0, 0, 0.f);

		RootSignature snowRootSignature{};
		snowRootSignature.Initialize(device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::Standard }
		);

		Shader snowVS{};
		snowVS.Intialize(L"Shader/Snow/VertexShader.hlsl", "main", "vs_5_1");

		Shader snowPS{};
		snowPS.Intialize(L"Shader/Snow/PixelShader.hlsl", "main", "ps_5_1");

		Shader snowGS{};
		snowGS.Intialize(L"Shader/Snow/GeometryShader.hlsl", "main", "gs_5_1");

		graphics_pipeline_state<vertex_layout<format<component_type::FLOAT,32,3>>,render_target_formats<FrameBufferFormat>> snowPipelineState{};
		snowPipelineState.Initialize(device, snowRootSignature, { &snowVS, &snowPS,&snowGS },
			{ "POSITION" }, false, true, PrimitiveTopology::PointList
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT depthViewport{ 0,0, static_cast<float>(MAP_RESOURCE_EDGE_SIZE),static_cast<float>(MAP_RESOURCE_EDGE_SIZE),0.f,1.f };
		D3D12_RECT depthScissorRect{ 0,0,static_cast<LONG>(MAP_RESOURCE_EDGE_SIZE),static_cast<LONG>(MAP_RESOURCE_EDGE_SIZE) };


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
		XMFLOAT3 lightDir{ -1.f,1.f,0.f };
		map(&sceneDataConstantBuffer, SceneData{ view,proj,eye,0.f,lightDir });

		XMFLOAT3 upPos{ 0.f,0.f,0.f };
		XMFLOAT3 upTarget{ 0,1,0 };
		XMFLOAT3 upUp{ 0,0,-1 };
		//ïΩçsìäâe
		auto upCamera = XMMatrixLookAtLH(XMLoadFloat3(&upPos), XMLoadFloat3(&upTarget), XMLoadFloat3(&upUp)) * XMMatrixOrthographicLH(GROUND_EDGE, GROUND_EDGE, -100.f, 100.f);
		map(&upCameraMatrixConstantBuffer, upCamera);

		XMFLOAT4 snowMove{};

		std::size_t cnt = 0;
		auto time = std::chrono::system_clock::now();

		while (UpdateWindow())
		{
			//
			//çXêV
			//

			if (cnt % 60 == 0) {
				auto now = std::chrono::system_clock::now();
				//std::cout << 60.f / static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now - time).count()) * 1000.f << "\n";
				time = now;
			}

			auto m1 = XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(30.f, 0.f, 0.f) * XMMatrixRotationY(cnt / 60.f) * XMMatrixTranslation(0.f, 0.f, 15.f);
			auto m2 = XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(-30.f, 0.f, 0.f) * XMMatrixRotationY(cnt / 60.f) * XMMatrixTranslation(0.f, 0.f, -15.f);
			map(&sphereDataConstantBuffer, SphereData{ {m1,m2} });

			snowMove.y -= 0.02f;
			if (snowMove.y < -SNOW_RANGE)
				snowMove.y += SNOW_RANGE * 2.f;
			map(&snowDataConstantBuffer, SnowData{ snowMove,XMFLOAT4{},SNOW_RANGE,1 / SNOW_RANGE,0.05f });

			cnt++;


			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);


			//
			//sphereÇÃdepthÇÃï`é 
			//

			command.Barrior(groundDepthShaderResource, resource_state::RenderTarget);
			command.Barrior(groundDepthBuffer, resource_state::DepthWrite);
			command.SetViewport(depthViewport);
			command.SetScissorRect(depthScissorRect);
			command.ClearRenderTargetView(groundDepthRTVDescriptorHeap.get_CPU_handle(), { 0.f });
			command.ClearDepthView(groundDepthStencilDescriptorHeap.get_CPU_handle(), 1.f);
			command.SetGraphicsRootSignature(sphereRootSignature);
			command.SetDescriptorHeap(sphereDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(sphereDepthPipelineState);
			command.SetVertexBuffer(sphereVertexBuffer);
			command.SetIndexBuffer(sphereIndexBuffer);
			command.SetRenderTarget(groundDepthRTVDescriptorHeap.get_CPU_handle(), groundDepthStencilDescriptorHeap.get_CPU_handle());
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(sphereFaceNum * 3, 2);
			command.Barrior(groundDepthShaderResource, resource_state::PixcelShaderResource);



			//
			//HeightMapÇÃåvéZ
			//

			command.Barrior(heightMapResource, resource_state::UnorderedAccessResource);
			command.SetComputeRootSignature(computeHeightRootSignature);
			command.SetDescriptorHeap(computeHeightDescriptorHeap);
			command.SetComputeRootDescriptorTable(0, computeHeightDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(computeHeightPipelineState);
			command.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.Barrior(heightMapResource, resource_state::PixcelShaderResource);


			//
			//NormalMapÇÃåvéZ
			//

			command.Barrior(normalMapResource, resource_state::UnorderedAccessResource);
			command.SetComputeRootSignature(computeNormalRootSignature);
			command.SetDescriptorHeap(computeNormalDescriptorHeap);
			command.SetComputeRootDescriptorTable(0, computeNormalDescriptorHeap.get_GPU_handle());
			command.SetPipelineState(computeNormalPipelineState);
			command.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.Barrior(normalMapResource, resource_state::PixcelShaderResource);



			//
			//ÉoÉbÉNÉoÉbÉtÉ@Ç÷ÇÃï`é 
			//

			command.Barrior(swapChain.GetFrameBuffer(backBufferIndex), resource_state::RenderTarget);
			command.Barrior(depthBuffer, resource_state::DepthWrite);

			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.ClearDepthView(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), depthStencilDescriptorHeap.get_CPU_handle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.SetPipelineState(groundPipelineState);
			command.SetGraphicsRootSignature(groundRootSignature);
			command.SetDescriptorHeap(groundDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.get_GPU_handle());
			command.SetVertexBuffer(vertexBuffer);
			command.SetIndexBuffer(indexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
			command.DrawIndexedInstanced(indexList.size());

			command.SetPipelineState(spherePipelineState);
			command.SetGraphicsRootSignature(sphereRootSignature);
			command.SetDescriptorHeap(sphereDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.get_GPU_handle());
			command.SetVertexBuffer(sphereVertexBuffer);
			command.SetIndexBuffer(sphereIndexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.DrawIndexedInstanced(sphereFaceNum * 3, 2);

			command.SetPipelineState(snowPipelineState);
			command.SetGraphicsRootSignature(snowRootSignature);
			command.SetDescriptorHeap(snowDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, snowDescriptorHeap.get_GPU_handle());
			command.SetVertexBuffer(snowVertexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::PointList);
			command.DrawInstanced(SNOW_NUM);


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