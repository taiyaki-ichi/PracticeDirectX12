#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"Shader.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/DepthBufferResource.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/ConstantBufferResource.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"Resource/IndexBufferResource.hpp"
#include"Resource/ShaderResource.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"

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

	struct Vertex {
		float x, y, z;
		float uvX, uvY;
	};

	struct Vertex2 {
		std::array<float, 3> pos;
		std::array<float, 3> normal;
	};

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

	inline std::pair<std::vector<Vertex>, std::vector<std::uint16_t>> GetGroundPatch()
	{
		std::vector<Vertex> vertexList{};
		std::vector<std::uint16_t> indexList{};

		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = GROUND_EDGE * x / DIVIDE;
				auto posZ = GROUND_EDGE * z / DIVIDE;
				vertexList.push_back(Vertex{ posX,0.f,posZ,posX / GROUND_EDGE,posZ / GROUND_EDGE });
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
			v.x -= GROUND_EDGE / 2.f;
			v.z -= GROUND_EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
	}


	inline int main()
	{
		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		Command<3> command{};
		command.Initialize(&device);

		auto swapChain = command.CreateSwapChain(&device, hwnd);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> rtvDescriptorHeap{};
		rtvDescriptorHeap.Initialize(&device, 3);
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(0));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(1));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(2));

		DepthBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);


		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));


		Shader groundVS{};
		groundVS.Intialize(L"Shader/Ground2/VertexShader.hlsl", "main", "vs_5_1");

		Shader grooundPS{};
		grooundPS.Intialize(L"Shader/Ground2/PixelShader.hlsl", "main", "ps_5_1");

		Shader groundHS{};
		groundHS.Intialize(L"Shader/Ground2/HullShader.hlsl", "main", "hs_5_1");

		Shader groundDS{};
		groundDS.Intialize(L"Shader/Ground2/DomainShader.hlsl", "main", "ds_5_1");

		RootSignature groundRootSignature{};
		groundRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV,DescriptorRangeType::SRV,
				DescriptorRangeType::SRV,DescriptorRangeType::SRV,
				DescriptorRangeType::SRV} },
			{ StaticSamplerType::Standard }
		);

		PipelineState groundPipelineState{};
		groundPipelineState.Initialize(&device, &groundRootSignature, { &groundVS, &grooundPS ,nullptr,&groundHS, &groundDS },
			{ {"POSITION",{Type::Float,3}},{"TEXCOOD",{Type::Float,2}} },
			{ {Type::UnsignedNormalizedInt8,4 } }, true, false, PrimitiveTopology::Patch
		);


		ConstantBufferResource groundDataConstantBufferResource{};
		groundDataConstantBufferResource.Initialize(&device, sizeof(GroundData));
		groundDataConstantBufferResource.Map(GroundData{ XMMatrixIdentity() });

		FloatShaderResource heightMapResource{};
		heightMapResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);

		FloatShaderResource elapsedTimeMapResource{};
		elapsedTimeMapResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);

		Float4ShaderResource normalMapResource{};
		normalMapResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);


		Float4ShaderResource groundDepthTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_DISP.png", &textureWidth, &textureHeight, &n, 4);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(textureWidth * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * textureHeight);
			uploadResource.Map(data, textureWidth * 4, textureHeight);
			groundDepthTextureResource.Initialize(&device, textureWidth, textureHeight);

			command.Reset(0);
			command.Barrior(&groundDepthTextureResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &groundDepthTextureResource);
			command.Barrior(&groundDepthTextureResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		Float4ShaderResource groundNormalTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/Snow_001_SD/Snow_001_NORM.jpg", &textureWidth, &textureHeight, &n, 4);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(textureWidth * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * textureHeight);
			uploadResource.Map(data, textureWidth * 4, textureHeight);
			groundNormalTextureResource.Initialize(&device, textureWidth, textureHeight);

			command.Reset(0);
			command.Barrior(&groundNormalTextureResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &groundNormalTextureResource);
			command.Barrior(&groundNormalTextureResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> groundDescriptorHeap{};
		groundDescriptorHeap.Initialize(&device, 7);
		groundDescriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);
		groundDescriptorHeap.PushBackView(&device, &groundDataConstantBufferResource);
		groundDescriptorHeap.PushBackView(&device, &heightMapResource);
		groundDescriptorHeap.PushBackView(&device, &normalMapResource);
		groundDescriptorHeap.PushBackView(&device, &groundDepthTextureResource);
		groundDescriptorHeap.PushBackView(&device, &groundNormalTextureResource);
		groundDescriptorHeap.PushBackView(&device, &elapsedTimeMapResource);

		auto [vertexList, indexList] = GetGroundPatch();

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(Vertex) * vertexList.size(), sizeof(Vertex));
		vertexBufferResource.Map(vertexList);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(std::uint16_t) * indexList.size());
		indexBufferResource.Map(indexList);


		FloatShaderResource groundDepthShaderResource{};
		groundDepthShaderResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 0.f);

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeHeightDescriptorHeap{};
		computeHeightDescriptorHeap.Initialize(&device, 3);
		computeHeightDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &heightMapResource);
		computeHeightDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &elapsedTimeMapResource);
		computeHeightDescriptorHeap.PushBackView(&device, &groundDepthShaderResource);

		Shader computeHeightCS{};
		computeHeightCS.Intialize(L"Shader/ComputeShader002.hlsl", "main", "cs_5_1");

		RootSignature computeHeightRootSignature{};
		computeHeightRootSignature.Initialize(&device,
			{ {DescriptorRangeType::UAV,DescriptorRangeType::UAV,DescriptorRangeType::SRV} },
			{}
		);

		PipelineState computeHeightPipelineState{};
		computeHeightPipelineState.Initialize(&device, &computeHeightRootSignature, &computeHeightCS);

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> computeNormalDescriptorHeap{};
		computeNormalDescriptorHeap.Initialize(&device, 2);
		computeNormalDescriptorHeap.PushBackView(&device, &heightMapResource);
		computeNormalDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &normalMapResource);

		Shader computeNormalCS{};
		computeNormalCS.Intialize(L"Shader/ComputeShader003.hlsl", "main", "cs_5_1");

		RootSignature computeNormalRootSignature{};
		computeNormalRootSignature.Initialize(&device,
			{ {DescriptorRangeType::SRV,DescriptorRangeType::UAV} },
			{}
		);

		PipelineState computeNormalPipelineState{};
		computeNormalPipelineState.Initialize(&device, &computeNormalRootSignature, &computeNormalCS);



		VertexBufferResource sphereVertexBufferResource{};
		IndexBufferResource sphereIndexBufferResource{};
		std::size_t sphereFaceNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);


			std::vector<Vertex2> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			sphereFaceNum = faceList.size();

			sphereVertexBufferResource.Initialize(&device, sizeof(Vertex2) * posNormalList.size(), sizeof(Vertex2));
			sphereVertexBufferResource.Map(std::move(posNormalList));

			sphereIndexBufferResource.Initialize(&device, sizeof(decltype(faceList)::value_type) * faceList.size());
			sphereIndexBufferResource.Map(std::move(faceList));
		}


		DepthBufferResource groundDepthStencilBufferResource{};
		groundDepthStencilBufferResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> groundDepthStencilDescriptorHeap{};
		groundDepthStencilDescriptorHeap.Initialize(&device, 1);
		groundDepthStencilDescriptorHeap.PushBackView(&device, &groundDepthStencilBufferResource);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> groundDepthRTVDescriptorHeap{};
		groundDepthRTVDescriptorHeap.Initialize(&device, 1);
		groundDepthRTVDescriptorHeap.PushBackView(&device, &groundDepthShaderResource);


		Shader sphereVS{};
		sphereVS.Intialize(L"Shader/Sphere/VertexShader.hlsl", "main", "vs_5_1");

		Shader spherePS{};
		spherePS.Intialize(L"Shader/Sphere/PixelShader.hlsl", "main", "ps_5_1");

		Shader sphereDepthVS{};
		sphereDepthVS.Intialize(L"Shader/Sphere/DepthVertexShader.hlsl", "main", "vs_5_1");

		Shader sphereDepthPS{};
		sphereDepthPS.Intialize(L"Shader/Sphere/DepthPixelShader.hlsl", "main", "ps_5_1");

		ConstantBufferResource sphereDataConstantBuffer{};
		sphereDataConstantBuffer.Initialize(&device, sizeof(SphereData));

		ConstantBufferResource upCameraMatrixConstantBuffer{};
		upCameraMatrixConstantBuffer.Initialize(&device, sizeof(XMMATRIX));

		RootSignature sphereRootSignature{};
		sphereRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::CBV} },
			{}
		);

		PipelineState spherePipelineState{};
		spherePipelineState.Initialize(&device, &sphereRootSignature, { &sphereVS, &spherePS },
			{ {"POSITION",{Type::Float,3}},{"NORMAL",{Type::Float,3}} },
			{ {Type::UnsignedNormalizedInt8,4} }, true, false, PrimitiveTopology::Triangle
		);

		PipelineState sphereDepthPipelineState{};
		sphereDepthPipelineState.Initialize(&device, &sphereRootSignature, { &sphereDepthVS, &sphereDepthPS },
			{ {"POSITION",{Type::Float,3}},{"NORMAL",{Type::Float,3} } },
			{ {Type::Float,1} }, true, false, PrimitiveTopology::Triangle
		);


		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> sphereDescriptorHeap{};
		sphereDescriptorHeap.Initialize(&device, 3);
		sphereDescriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);
		sphereDescriptorHeap.PushBackView(&device, &sphereDataConstantBuffer);
		sphereDescriptorHeap.PushBackView(&device, &upCameraMatrixConstantBuffer);


		constexpr std::size_t SNOW_NUM = 8000;
		constexpr float SNOW_RANGE = 8.f;
		VertexBufferResource snowVertexBufferResource{};
		{
			std::random_device seed_gen;
			std::default_random_engine engine(seed_gen());
			std::uniform_real_distribution<float> dist(-SNOW_RANGE, SNOW_RANGE);
			std::vector<std::array<float, 3>> v{};
			v.reserve(SNOW_NUM);
			for (std::size_t i = 0; i < SNOW_NUM; i++)
				v.emplace_back(std::array<float, 3>{dist(engine), dist(engine), dist(engine)});
			snowVertexBufferResource.Initialize(&device, sizeof(float) * 3 * SNOW_NUM, sizeof(float) * 3);
			snowVertexBufferResource.Map(std::move(v));
		}

		ConstantBufferResource snowDataConstantBufferResource{};
		snowDataConstantBufferResource.Initialize(&device, sizeof(SnowData));


		Float4ShaderResource snowTextureShaderResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/snow.png", &textureWidth, &textureHeight, &n, 4);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(textureWidth * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * textureHeight);
			uploadResource.Map(data, textureWidth * 4, textureHeight);
			snowTextureShaderResource.Initialize(&device, textureWidth, textureHeight);

			command.Reset(0);
			command.Barrior(&snowTextureShaderResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &snowTextureShaderResource);
			command.Barrior(&snowTextureShaderResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}


		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> snowDescriptorHeap{};
		snowDescriptorHeap.Initialize(&device, 3);
		snowDescriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);
		snowDescriptorHeap.PushBackView(&device, &snowDataConstantBufferResource);
		snowDescriptorHeap.PushBackView(&device, &snowTextureShaderResource);

		RootSignature snowRootSignature{};
		snowRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::Standard }
		);

		Shader snowVS{};
		snowVS.Intialize(L"Shader/Snow/VertexShader.hlsl", "main", "vs_5_1");

		Shader snowPS{};
		snowPS.Intialize(L"Shader/Snow/PixelShader.hlsl", "main", "ps_5_1");

		Shader snowGS{};
		snowGS.Intialize(L"Shader/Snow/GeometryShader.hlsl", "main", "gs_5_1");

		PipelineState snowPipelineState{};
		snowPipelineState.Initialize(&device, &snowRootSignature, { &snowVS, &snowPS,&snowGS },
			{ {"POSITION",{Type::Float,3}} }, { {Type::UnsignedNormalizedInt8,4} },
			false, true, PrimitiveTopology::PointList
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
		sceneDataConstantBufferResource.Map(SceneData{ view,proj,eye,0.f,lightDir });

		XMFLOAT3 upPos{ 0.f,0.f,0.f };
		XMFLOAT3 upTarget{ 0,1,0 };
		XMFLOAT3 upUp{ 0,0,-1 };
		//ïΩçsìäâe
		auto upCamera = XMMatrixLookAtLH(XMLoadFloat3(&upPos), XMLoadFloat3(&upTarget), XMLoadFloat3(&upUp)) * XMMatrixOrthographicLH(GROUND_EDGE, GROUND_EDGE, -100.f, 100.f);
		upCameraMatrixConstantBuffer.Map(upCamera);

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
			sphereDataConstantBuffer.Map(SphereData{ {m1,m2} });

			snowMove.y -= 0.02f;
			if (snowMove.y < -SNOW_RANGE)
				snowMove.y += SNOW_RANGE * 2.f;
			snowDataConstantBufferResource.Map(SnowData{ snowMove,XMFLOAT4{},SNOW_RANGE,1 / SNOW_RANGE,0.05f });

			cnt++;


			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			//
			//sphereÇÃdepthÇÃï`é 
			//

			command.Barrior(&groundDepthShaderResource, ResourceState::RenderTarget);
			command.SetViewport(depthViewport);
			command.SetScissorRect(depthScissorRect);
			command.ClearRenderTargetView(groundDepthRTVDescriptorHeap.GetCPUHandle(), { 0.f });
			command.ClearDepthView(groundDepthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			command.SetGraphicsRootSignature(&sphereRootSignature);
			command.SetDescriptorHeap(&sphereDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&sphereDepthPipelineState);
			command.SetVertexBuffer(&sphereVertexBufferResource);
			command.SetIndexBuffer(&sphereIndexBufferResource);
			command.SetRenderTarget(groundDepthRTVDescriptorHeap.GetCPUHandle(), groundDepthStencilDescriptorHeap.GetCPUHandle());
			command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			command.DrawIndexedInstanced(sphereFaceNum * 3, 2);
			command.Barrior(&groundDepthShaderResource, ResourceState::PixcelShaderResource);



			//
			//HeightMapÇÃåvéZ
			//

			command.Barrior(&heightMapResource, ResourceState::UnorderedAccessResource);
			command.SetComputeRootSignature(&computeHeightRootSignature);
			command.SetDescriptorHeap(&computeHeightDescriptorHeap);
			command.SetComputeRootDescriptorTable(0, computeHeightDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&computeHeightPipelineState);
			command.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.Barrior(&heightMapResource, ResourceState::PixcelShaderResource);


			//
			//NormalMapÇÃåvéZ
			//

			command.Barrior(&normalMapResource, ResourceState::UnorderedAccessResource);
			command.SetComputeRootSignature(&computeNormalRootSignature);
			command.SetDescriptorHeap(&computeNormalDescriptorHeap);
			command.SetComputeRootDescriptorTable(0, computeNormalDescriptorHeap.GetGPUHandle());
			command.SetPipelineState(&computeNormalPipelineState);
			command.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			command.Barrior(&normalMapResource, ResourceState::PixcelShaderResource);



			//
			//ÉoÉbÉNÉoÉbÉtÉ@Ç÷ÇÃï`é 
			//

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);

			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), depthStencilDescriptorHeap.GetCPUHandle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.SetPipelineState(&groundPipelineState);
			command.SetGraphicsRootSignature(&groundRootSignature);
			command.SetDescriptorHeap(&groundDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, groundDescriptorHeap.GetGPUHandle());
			command.SetVertexBuffer(&vertexBufferResource);
			command.SetIndexBuffer(&indexBufferResource);
			command.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
			command.DrawIndexedInstanced(indexList.size());

			command.SetPipelineState(&spherePipelineState);
			command.SetGraphicsRootSignature(&sphereRootSignature);
			command.SetDescriptorHeap(&sphereDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.GetGPUHandle());
			command.SetVertexBuffer(&sphereVertexBufferResource);
			command.SetIndexBuffer(&sphereIndexBufferResource);
			command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			command.DrawIndexedInstanced(sphereFaceNum * 3, 2);

			command.SetPipelineState(&snowPipelineState);
			command.SetGraphicsRootSignature(&snowRootSignature);
			command.SetDescriptorHeap(&snowDescriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, snowDescriptorHeap.GetGPUHandle());
			command.SetVertexBuffer(&snowVertexBufferResource);
			command.SetPrimitiveTopology(PrimitiveTopology::PointList);
			command.DrawInstanced(SNOW_NUM);


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