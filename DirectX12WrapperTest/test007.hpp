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
#include"OffLoader.hpp"
#include"utility.hpp"

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

	struct SceneData2 {
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT3 eye;
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
		computeHeightDescriptorHeap.Initialize(&device, 3);

		computeHeightDescriptorHeap.PushBackView<DescriptorHeapViewTag::UnorderedAccessResource>(&device, &heightMapResource);

		Shader cs{};
		cs.Intialize(L"Shader/ComputeShader002.hlsl", "main", "cs_5_0");

		RootSignature computeHeightRootSignature{};
		computeHeightRootSignature.Initialize(&device,
			{ {DescriptorRangeType::UAV,DescriptorRangeType::SRV} },
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




		FloatShaderResource groundDepthShaderResource{};
		groundDepthShaderResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE, 0.f);

		computeHeightDescriptorHeap.PushBackView(&device, &groundDepthShaderResource);

		DepthStencilBufferResource groundDepthStencilBufferResource{};
		groundDepthStencilBufferResource.Initialize(&device, MAP_RESOURCE_EDGE_SIZE, MAP_RESOURCE_EDGE_SIZE);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> groundDepthStencilDescriptorHeap{};
		groundDepthStencilDescriptorHeap.Initialize(&device, 1);
		groundDepthStencilDescriptorHeap.PushBackView(&device,&groundDepthStencilBufferResource);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> groundDepthRTVDescriptorHeap{};
		groundDepthRTVDescriptorHeap.Initialize(&device, 1);
		groundDepthRTVDescriptorHeap.PushBackView(&device, &groundDepthShaderResource);
		

		Shader svs{};
		svs.Intialize(L"Shader/Sphere/VertexShader.hlsl", "main", "vs_5_1");

		Shader sps{};
		sps.Intialize(L"Shader/Sphere/PixelShader.hlsl", "main", "ps_5_1");

		Shader dvs{};
		dvs.Intialize(L"Shader/Sphere/DepthVertexShader.hlsl", "main", "vs_5_1");

		Shader dps{};
		dps.Intialize(L"Shader/Sphere/DepthPixelShader.hlsl", "main", "ps_5_1");

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
		spherePipelineState.Initialize(&device, &sphereRootSignature, &svs, &sps,
			{ {"POSITION",VertexLayoutFormat::Float3},{"NORMAL",VertexLayoutFormat::Float3} },
			{ Format::R8G8B8A8 }, true);

		PipelineState sphereDepthPipelineState{};
		sphereDepthPipelineState.Initialize(&device, &sphereRootSignature, &dvs, &dps,
			{ {"POSITION",VertexLayoutFormat::Float3},{"NORMAL",VertexLayoutFormat::Float3} },
			{ Format::R32_FLOAT }, true);


		ConstantBufferResource sceneData2ConstantBufferResource{};
		sceneData2ConstantBufferResource.Initialize(&device, sizeof(SceneData2));


		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> sphereDescriptorHeap{};
		sphereDescriptorHeap.Initialize(&device, 3);
		sphereDescriptorHeap.PushBackView(&device, &sceneData2ConstantBufferResource);
		sphereDescriptorHeap.PushBackView(&device, &sphereDataConstantBuffer);
		sphereDescriptorHeap.PushBackView(&device, &upCameraMatrixConstantBuffer);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT depthViewport{ 0,0, static_cast<float>(MAP_RESOURCE_EDGE_SIZE),static_cast<float>(MAP_RESOURCE_EDGE_SIZE),0.f,1.f };
		D3D12_RECT depthScissorRect{ 0,0,static_cast<LONG>(MAP_RESOURCE_EDGE_SIZE),static_cast<LONG>(MAP_RESOURCE_EDGE_SIZE) };


		XMFLOAT3 eye{ 40, 20.f, 0.f };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			500.f
		);

		sceneDataConstantBufferResource.Map(SceneData{ view,proj,
			XMMatrixIdentity(),
			eye,XMFLOAT4(16.f,64.f,4.f,0.f) });
		sceneData2ConstantBufferResource.Map(SceneData2{ view,proj,eye });

		XMFLOAT3 pos{ 0.f,0.f,0.f };
		XMFLOAT3 u{ 0,0,-1 };
		XMFLOAT3 t{ 0,1,0 };
		//平行投影
		auto upCamera = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&t), XMLoadFloat3(&u)) * XMMatrixOrthographicLH(GROUND_EDGE, GROUND_EDGE, -100.f, 100.f);
		upCameraMatrixConstantBuffer.Map(upCamera);


		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			//
			//更新
			//

			auto m1 = XMMatrixScaling(5.f, 5.f, 5.f) * XMMatrixTranslation(30.f, 0.f, 0.f) * XMMatrixRotationY(cnt / 50.f) * XMMatrixTranslation(0.f, 0.f, 10.f);
			auto m2 = XMMatrixScaling(5.f, 5.f, 5.f) * XMMatrixTranslation(-30.f, 0.f, 0.f) * XMMatrixRotationY(cnt / 50.f) * XMMatrixTranslation(0.f, 0.f, -10.f);
			sphereDataConstantBuffer.Map(SphereData{ {m1,m2} });

			cnt++;


			//
			//sphereのdepthの描写
			//

			commandList.Barrior(&groundDepthShaderResource, ResourceState::RenderTarget);
			commandList.SetViewport(depthViewport);
			commandList.SetScissorRect(depthScissorRect);
			commandList.ClearRenderTargetView(groundDepthRTVDescriptorHeap.GetCPUHandle(), { 0.f });
			commandList.ClearDepthView(groundDepthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			commandList.SetGraphicsRootSignature(&sphereRootSignature);
			commandList.SetDescriptorHeap(&sphereDescriptorHeap);
			commandList.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.GetGPUHandle());
			commandList.SetPipelineState(&sphereDepthPipelineState);
			commandList.SetVertexBuffer(&sphereVertexBufferResource);
			commandList.SetIndexBuffer(&sphereIndexBufferResource);
			commandList.SetRenderTarget(groundDepthRTVDescriptorHeap.GetCPUHandle(), groundDepthStencilDescriptorHeap.GetCPUHandle());
			commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			commandList.DrawIndexedInstanced(sphereFaceNum * 3, 2);
			commandList.Barrior(&groundDepthShaderResource, ResourceState::PixcelShaderResource);



			//
			//HeightMapの計算
			//

			commandList.Barrior(&heightMapResource, ResourceState::UnorderedAccessResource);
			commandList.SetComputeRootSignature(&computeHeightRootSignature);
			commandList.SetDescriptorHeap(&computeHeightDescriptorHeap);
			commandList.SetComputeRootDescriptorTable(0, computeHeightDescriptorHeap.GetGPUHandle());
			commandList.SetPipelineState(&computeHeightPipelineState);
			commandList.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			commandList.Barrior(&heightMapResource, ResourceState::PixcelShaderResource);


			//
			//NormalMapの計算
			//

			commandList.Barrior(&normalMapResource, ResourceState::UnorderedAccessResource);
			commandList.SetComputeRootSignature(&computeNormalRootSignature);
			commandList.SetDescriptorHeap(&computeNormalDescriptorHeap);
			commandList.SetComputeRootDescriptorTable(0, computeNormalDescriptorHeap.GetGPUHandle());
			commandList.SetPipelineState(&computeNormalPipelineState);
			commandList.Dispatch(MAP_RESOURCE_EDGE_SIZE / 8 + 1, MAP_RESOURCE_EDGE_SIZE / 8 + 1, 1);
			commandList.Barrior(&normalMapResource, ResourceState::PixcelShaderResource);



			//
			//バックバッファへの描写
			//

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::RenderTarget);

			commandList.ClearBackBuffer(&doubleBuffer);
			commandList.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(), depthStencilDescriptorHeap.GetCPUHandle());
			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);


			commandList.SetPipelineState(&pipelineState);
			commandList.SetGraphicsRootSignature(&rootSignature);
			commandList.SetDescriptorHeap(&descriptorHeap);
			commandList.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			commandList.SetVertexBuffer(&vertexBufferResource);
			commandList.SetIndexBuffer(&indexBufferResource);
			commandList.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
			commandList.DrawIndexedInstanced(indexList.size());

			commandList.SetPipelineState(&spherePipelineState);
			commandList.SetGraphicsRootSignature(&sphereRootSignature);
			commandList.SetDescriptorHeap(&sphereDescriptorHeap);
			commandList.SetGraphicsRootDescriptorTable(0, sphereDescriptorHeap.GetGPUHandle());
			commandList.SetVertexBuffer(&sphereVertexBufferResource);
			commandList.SetIndexBuffer(&sphereIndexBufferResource);
			commandList.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			commandList.DrawIndexedInstanced(sphereFaceNum * 3, 2);

			commandList.BarriorToBackBuffer(&doubleBuffer, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		}

		return 0;
	}



}