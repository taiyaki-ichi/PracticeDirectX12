#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"shader.hpp"
#include"root_signature.hpp"
#include"pipeline_state.hpp"
#include"descriptor_heap.hpp"
#include"resource/constant_buffer_resource.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"
#include"resource/shader_resource.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"
#include"resource/texture_upload_buffer_resource.hpp"

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

	using VertexLayout1 = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;
	using VertexLayout2 = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3>>;

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

	inline std::pair<std::vector<std::array<float,5>>, std::vector<std::uint32_t>> GetGroundPatch()
	{
		std::vector<std::array<float, 5>> vertexList{};
		std::vector<std::uint32_t> indexList{};

		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = GROUND_EDGE * x / DIVIDE;
				auto posZ = GROUND_EDGE * z / DIVIDE;
				vertexList.push_back({ posX,0.f,posZ,posX / GROUND_EDGE,posZ / GROUND_EDGE });
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
			v[0] -= GROUND_EDGE / 2.f;
			v[2] -= GROUND_EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
	}


	inline int main()
	{
		auto hwnd = create_simple_window(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		device device{};
		device.initialize();

		command<FRAME_BUFFER_NUM> command{};
		command.initialize(device);

		swap_chain<FrameBufferFormat, FRAME_BUFFER_NUM> swapChain{};
		swapChain.initialize(command, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, FRAME_BUFFER_NUM);
		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(i), 0, 0);

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::ALLOW_DEPTH_STENCIL> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { {1.f} });

		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(device, 1);
		depthStencilDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);


		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);


		shader groundVS{};
		groundVS.initialize(L"Shader/Ground2/VertexShader.hlsl", "main", "vs_5_1");

		shader grooundPS{};
		grooundPS.initialize(L"Shader/Ground2/PixelShader.hlsl", "main", "ps_5_1");

		shader groundHS{};
		groundHS.initialize(L"Shader/Ground2/HullShader.hlsl", "main", "hs_5_1");

		shader groundDS{};
		groundDS.initialize(L"Shader/Ground2/DomainShader.hlsl", "main", "ds_5_1");

		root_signature groundRootSignature{};
		groundRootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::CBV,descriptor_range_type::SRV,descriptor_range_type::SRV,
				descriptor_range_type::SRV,descriptor_range_type::SRV,
				descriptor_range_type::SRV} },
			{ static_sampler_desc::wrap_point() }
		);

		graphics_pipeline_state<VertexLayout1,format_tuple<FrameBufferFormat>> groundPipelineState{};
		groundPipelineState.initialize(device, groundRootSignature, { &groundVS, &grooundPS ,nullptr,&groundHS, &groundDS },
			{ "POSITION","TEXCOOD" }, true, false, primitive_topology::PATCH
		);


		constant_buffer_resource<GroundData> groundDataConstantBuffer{};
		groundDataConstantBuffer.initialize(device);

		auto groundDataMappedResource = map(groundDataConstantBuffer);
		groundDataMappedResource.reference() = { XMMatrixIdentity() };
		

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::ALLOW_UNORDERED_ACCESS> heightMapResource{};
		heightMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);

		shader_resource<format<component_type::UINT, 32, 1>, resource_flag::ALLOW_UNORDERED_ACCESS> elapsedTimeMapResource{};
		elapsedTimeMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>, resource_flag::ALLOW_UNORDERED_ACCESS> normalMapResource{};
		normalMapResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1);


		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> groundDepthTextureResource{};
		{
			int x, y, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_DISP.png", &x, &y, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, x, y);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < y; i++)
				for (std::uint32_t j = 0; j < x; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * y) * 4 + k];

			groundDepthTextureResource.initialize(device, x, y, 1, 1);

			command.reset(0);
			command.barrior(groundDepthTextureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, groundDepthTextureResource);
			command.barrior(groundDepthTextureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

			stbi_image_free(data);
		}

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> groundNormalTextureResource{};
		{
			int x, y, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_NORM.jpg", &x, &y, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, x, y);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < y; i++)
				for (std::uint32_t j = 0; j < x; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * y) * 4 + k];

			groundNormalTextureResource.initialize(device, x, y, 1, 1);

			command.reset(0);
			command.barrior(groundNormalTextureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, groundNormalTextureResource);
			command.barrior(groundNormalTextureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

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

		vertex_buffer_resource<VertexLayout1> vertexBuffer{};
		vertexBuffer.initialize(device, vertexList.size());

		auto vertexBufferMappedResource = map(vertexBuffer);
		for (std::uint32_t i = 0; i < vertexList.size(); i++)
		{
			for (std::uint32_t j = 0; j < 3; j++)
				vertexBufferMappedResource.reference<0>(i, j) = vertexList[i][j];
			for (std::uint32_t j = 0; j < 2; j++)
				vertexBufferMappedResource.reference<1>(i, j) = vertexList[i][j + 3];
		}


		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, indexList.size());

		auto indexBufferMappedResource = map(indexBuffer);
		for (std::uint32_t i = 0; i < indexList.size(); i++)
			indexBufferMappedResource.reference(i) = indexList[i];


		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::ALLOW_RENDER_TARGET> groundDepthShaderResource{};
		groundDepthShaderResource.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1, { {0.f} });

		descriptor_heap_CBV_SRV_UAV computeHeightDescriptorHeap{};
		computeHeightDescriptorHeap.initialize(device, 3);
		computeHeightDescriptorHeap.push_back_texture2D_UAV(device, heightMapResource, 0, 0);
		computeHeightDescriptorHeap.push_back_texture2D_UAV(device, elapsedTimeMapResource, 0, 0);
		computeHeightDescriptorHeap.push_back_texture2D_SRV(device, groundDepthShaderResource, 1, 0, 0, 0.f);

		shader computeHeightCS{};
		computeHeightCS.initialize(L"Shader/ComputeShader002.hlsl", "main", "cs_5_1");

		root_signature computeHeightRootSignature{};
		computeHeightRootSignature.initialize(device,
			{ {descriptor_range_type::UAV,descriptor_range_type::UAV,descriptor_range_type::SRV} },
			{}
		);

		compute_pipeline_state computeHeightPipelineState{};
		computeHeightPipelineState.initialize(device, computeHeightRootSignature, computeHeightCS);

		descriptor_heap_CBV_SRV_UAV computeNormalDescriptorHeap{};
		computeNormalDescriptorHeap.initialize(device, 2);
		computeNormalDescriptorHeap.push_back_texture2D_SRV(device, heightMapResource, 1, 0, 0, 0.f);
		computeNormalDescriptorHeap.push_back_texture2D_UAV(device, normalMapResource, 0, 0);

		shader computeNormalCS{};
		computeNormalCS.initialize(L"Shader/ComputeShader003.hlsl", "main", "cs_5_1");

		root_signature computeNormalRootSignature{};
		computeNormalRootSignature.initialize(device,
			{ {descriptor_range_type::SRV,descriptor_range_type::UAV} },
			{}
		);

		compute_pipeline_state computeNormalPipelineState{};
		computeNormalPipelineState.initialize(device, computeNormalRootSignature, computeNormalCS);


		vertex_buffer_resource<VertexLayout2> sphereVertexBuffer{};
		index_buffer_resource<format<component_type::UINT,32,1>> sphereIndexBuffer{};
		std::size_t sphereFaceNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);

			sphereVertexBuffer.initialize(device, vertexList.size());
			auto sphereVertexBufferMappedResource = map(sphereVertexBuffer);
			XMFLOAT3 tmpFloat3;
			for (std::uint32_t i = 0; i < vertexList.size(); i++)
			{
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				for (std::uint32_t j = 0; j < 3; j++)
					sphereVertexBufferMappedResource.reference<0>(i, j) = vertexList[i][j];
				sphereVertexBufferMappedResource.reference<1>(i, 0) = tmpFloat3.x;
				sphereVertexBufferMappedResource.reference<1>(i, 1) = tmpFloat3.y;
				sphereVertexBufferMappedResource.reference<1>(i, 2) = tmpFloat3.z;
			}

			sphereFaceNum = faceList.size();

			sphereIndexBuffer.initialize(device, faceList.size() * 3);

			auto sphereIndexBufferMappedResource = map(sphereIndexBuffer);
			for (std::uint32_t i = 0; i < faceList.size(); i++)
				for (std::uint32_t j = 0; j < 3; j++)
					sphereIndexBufferMappedResource.reference(i * 3 + j) = faceList[i][j];

		}


		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::ALLOW_DEPTH_STENCIL> groundDepthBuffer{};
		groundDepthBuffer.initialize(device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 1, 1, { {1.f} });

		descriptor_heap_DSV groundDepthStencilDescriptorHeap{};
		groundDepthStencilDescriptorHeap.initialize(device, 1);
		groundDepthStencilDescriptorHeap.push_back_texture2D_DSV(device, groundDepthBuffer, 0);

		descriptor_heap_RTV groundDepthRTVDescriptorHeap{};
		groundDepthRTVDescriptorHeap.initialize(device, 1);
		groundDepthRTVDescriptorHeap.push_back_texture2D_RTV(device, groundDepthShaderResource, 0, 0);


		shader sphereVS{};
		sphereVS.initialize(L"Shader/Sphere/VertexShader.hlsl", "main", "vs_5_1");

		shader spherePS{};
		spherePS.initialize(L"Shader/Sphere/PixelShader.hlsl", "main", "ps_5_1");

		shader sphereDepthVS{};
		sphereDepthVS.initialize(L"Shader/Sphere/DepthVertexShader.hlsl", "main", "vs_5_1");

		shader sphereDepthPS{};
		sphereDepthPS.initialize(L"Shader/Sphere/DepthPixelShader.hlsl", "main", "ps_5_1");

		constant_buffer_resource<SphereData> sphereDataConstantBuffer{};
		sphereDataConstantBuffer.initialize(device);

		constant_buffer_resource<XMMATRIX> upCameraMatrixConstantBuffer{};
		upCameraMatrixConstantBuffer.initialize(device);

		root_signature sphereRootSignature{};
		sphereRootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::CBV,descriptor_range_type::CBV} },
			{}
		);

		graphics_pipeline_state<VertexLayout2,format_tuple<FrameBufferFormat>> spherePipelineState{};
		spherePipelineState.initialize(device, sphereRootSignature, { &sphereVS, &spherePS },
			{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
		);

		graphics_pipeline_state<VertexLayout2,format_tuple<format<component_type::FLOAT,32,1>>> sphereDepthPipelineState{};
		sphereDepthPipelineState.initialize(device, sphereRootSignature, { &sphereDepthVS, &sphereDepthPS },
			{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
		);


		descriptor_heap_CBV_SRV_UAV sphereDescriptorHeap{};
		sphereDescriptorHeap.initialize(device, 3);
		sphereDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		sphereDescriptorHeap.push_back_CBV(device, sphereDataConstantBuffer);
		sphereDescriptorHeap.push_back_CBV(device, upCameraMatrixConstantBuffer);


		constexpr std::size_t SNOW_NUM = 8000;
		constexpr float SNOW_RANGE = 8.f;
		vertex_buffer_resource<format_tuple<format<component_type::FLOAT,32,3>>> snowVertexBuffer{};
		{
			std::random_device seed_gen;
			std::default_random_engine engine(seed_gen());
			std::uniform_real_distribution<float> dist(-SNOW_RANGE, SNOW_RANGE);

			snowVertexBuffer.initialize(device, SNOW_NUM);
			auto snowVertexBufferMappedResource = map(snowVertexBuffer);
			for (std::uint32_t i = 0; i < SNOW_NUM; i++)
				for (std::uint32_t j = 0; j < 3; j++)
					snowVertexBufferMappedResource.reference(i, j) = dist(engine);
		}

		constant_buffer_resource<SnowData> snowDataConstantBuffer{};
		snowDataConstantBuffer.initialize(device);


		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> snowTextureShader{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/snow.png", &textureWidth, &textureHeight, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, textureWidth, textureHeight);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < textureHeight; i++)
				for (std::uint32_t j = 0; j < textureWidth; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * textureWidth) * 4 + k];

			snowTextureShader.initialize(device, textureWidth, textureHeight, 1, 1);

			command.reset(0);
			command.barrior(snowTextureShader, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, snowTextureShader);
			command.barrior(snowTextureShader, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

			stbi_image_free(data);
		}


		descriptor_heap_CBV_SRV_UAV snowDescriptorHeap{};
		snowDescriptorHeap.initialize(device, 3);
		snowDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		snowDescriptorHeap.push_back_CBV(device, snowDataConstantBuffer);
		snowDescriptorHeap.push_back_texture2D_SRV(device, snowTextureShader, 1, 0, 0, 0.f);

		root_signature snowRootSignature{};
		snowRootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::CBV,descriptor_range_type::SRV} },
			{ static_sampler_desc::clamp_anisotropic() }
		);

		shader snowVS{};
		snowVS.initialize(L"Shader/Snow/VertexShader.hlsl", "main", "vs_5_1");

		shader snowPS{};
		snowPS.initialize(L"Shader/Snow/PixelShader.hlsl", "main", "ps_5_1");

		shader snowGS{};
		snowGS.initialize(L"Shader/Snow/GeometryShader.hlsl", "main", "gs_5_1");

		graphics_pipeline_state<format_tuple<format<component_type::FLOAT,32,3>>,format_tuple<FrameBufferFormat>> snowPipelineState{};
		snowPipelineState.initialize(device, snowRootSignature, { &snowVS, &snowPS,&snowGS },
			{ "POSITION" }, false, true, primitive_topology::POINT_LIST
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

		auto sceneDataMappedResource = map(sceneDataConstantBuffer);
		sceneDataMappedResource.reference() = { view,proj,eye,0.f,lightDir };


		XMFLOAT3 upPos{ 0.f,0.f,0.f };
		XMFLOAT3 upTarget{ 0,1,0 };
		XMFLOAT3 upUp{ 0,0,-1 };
		//ïΩçsìäâe
		auto upCamera = XMMatrixLookAtLH(XMLoadFloat3(&upPos), XMLoadFloat3(&upTarget), XMLoadFloat3(&upUp)) * XMMatrixOrthographicLH(GROUND_EDGE, GROUND_EDGE, -100.f, 100.f);
		
		auto upCameraMatrixMappedResource = map(upCameraMatrixConstantBuffer);
		upCameraMatrixMappedResource.reference() = upCamera;

		XMFLOAT4 snowMove{};

		auto sphereDataMappedResource = map(sphereDataConstantBuffer);

		auto snowDataMappedResource = map(snowDataConstantBuffer);
		snowDataMappedResource.reference().center = {};
		snowDataMappedResource.reference().range = SNOW_RANGE;
		snowDataMappedResource.reference().rangeR = 1 / SNOW_RANGE;
		snowDataMappedResource.reference().size = 0.05f;

		std::size_t cnt = 0;
		auto time = std::chrono::system_clock::now();

		while (update_window())
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
			sphereDataMappedResource.reference() = { m1,m2 };

			snowMove.y -= 0.02f;
			if (snowMove.y < -SNOW_RANGE)
				snowMove.y += SNOW_RANGE * 2.f;
			snowDataMappedResource.reference().move = snowMove;

			cnt++;


			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);


			//
			//sphereÇÃdepthÇÃï`é 
			//

			command.barrior(groundDepthShaderResource, resource_state::RenderTarget);
			command.barrior(groundDepthBuffer, resource_state::DepthWrite);
			command.set_viewport(depthViewport);
			command.set_scissor_rect(depthScissorRect);
			command.clear_render_target_view(groundDepthRTVDescriptorHeap.get_CPU_handle(), { 0.f });
			command.clear_depth_view(groundDepthStencilDescriptorHeap.get_CPU_handle(), 1.f);
			command.set_graphics_root_signature(sphereRootSignature);
			command.set_descriptor_heap(sphereDescriptorHeap);
			command.set_graphics_root_descriptor_table(0, sphereDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(sphereDepthPipelineState);
			command.set_vertex_buffer(sphereVertexBuffer);
			command.set_index_buffer(sphereIndexBuffer);
			command.set_render_target({ {groundDepthRTVDescriptorHeap.get_CPU_handle()} }, groundDepthStencilDescriptorHeap.get_CPU_handle());
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(sphereFaceNum * 3, 2);
			command.barrior(groundDepthShaderResource, resource_state::PixcelShaderResource);



			//
			//HeightMapÇÃåvéZ
			//

			command.barrior(heightMapResource, resource_state::UnorderedAccessResource);
			command.set_compute_root_signature(computeHeightRootSignature);
			command.set_descriptor_heap(computeHeightDescriptorHeap);
			command.set_compute_root_descriptor_table(0, computeHeightDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(computeHeightPipelineState);
			command.dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.barrior(heightMapResource, resource_state::PixcelShaderResource);


			//
			//NormalMapÇÃåvéZ
			//

			command.barrior(normalMapResource, resource_state::UnorderedAccessResource);
			command.set_compute_root_signature(computeNormalRootSignature);
			command.set_descriptor_heap(computeNormalDescriptorHeap);
			command.set_compute_root_descriptor_table(0, computeNormalDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(computeNormalPipelineState);
			command.dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.barrior(normalMapResource, resource_state::PixcelShaderResource);



			//
			//ÉoÉbÉNÉoÉbÉtÉ@Ç÷ÇÃï`é 
			//

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.barrior(depthBuffer, resource_state::DepthWrite);

			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.clear_depth_view(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);
			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} }, depthStencilDescriptorHeap.get_CPU_handle());
			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.set_pipeline_state(groundPipelineState);
			command.set_graphics_root_signature(groundRootSignature);
			command.set_descriptor_heap(groundDescriptorHeap);
			command.set_graphics_root_descriptor_table(0, groundDescriptorHeap.get_GPU_handle());
			command.set_vertex_buffer(vertexBuffer);
			command.set_index_buffer(indexBuffer);
			command.set_primitive_topology(primitive_topology::CONTOROL_4_POINT_PATCH_LIST);
			command.draw_indexed_instanced(indexList.size());

			command.set_pipeline_state(spherePipelineState);
			command.set_graphics_root_signature(sphereRootSignature);
			command.set_descriptor_heap(sphereDescriptorHeap);
			command.set_graphics_root_descriptor_table(0, sphereDescriptorHeap.get_GPU_handle());
			command.set_vertex_buffer(sphereVertexBuffer);
			command.set_index_buffer(sphereIndexBuffer);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(sphereFaceNum * 3, 2);

			command.set_pipeline_state(snowPipelineState);
			command.set_graphics_root_signature(snowRootSignature);
			command.set_descriptor_heap(snowDescriptorHeap);
			command.set_graphics_root_descriptor_table(0, snowDescriptorHeap.get_GPU_handle());
			command.set_vertex_buffer(snowVertexBuffer);
			command.set_primitive_topology(primitive_topology::POINT_LIST);
			command.draw_instanced(SNOW_NUM);


			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);

			command.close();
			command.execute();

			swapChain.present();
			command.fence(backBufferIndex);

			command.wait(swapChain.get_vcurrent_back_buffer_index());
		}

		command.wait_all(device);
		return 0;
	}

}