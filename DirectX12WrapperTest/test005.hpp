#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"
#include"Resource/VertexBufferResource.hpp"

#include<array>
#include<DirectXMath.h>


namespace test005
{
	using namespace DX12;
	using namespace DirectX;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct SceneData {
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 lightDir;
		XMFLOAT3 eye;
	};

	struct ColorObjectData {
		XMMATRIX world;
		std::array<float, 4> color;
	};

	constexpr std::size_t CUBE_MAP_EDGE = 512;

	struct CubemapSceneData {
		XMMATRIX view[6];
		XMMATRIX proj;
	};

	class Mesh
	{
		std::size_t faceNum{};

		VertexBufferResource vertexBufferResource{};
		IndexBufferResource indexBufferResource{};

		struct Vertex {
			std::array<float, 3> pos;
			std::array<float, 3> normal;
		};

	public:
		void Initialize(Device* device,const char* fileName)
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>(fileName);
			auto normalList = GetVertexNormal(vertexList, faceList);

			std::vector<Vertex> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			faceNum = faceList.size();

			vertexBufferResource.Initialize(device, sizeof(Vertex) * posNormalList.size(), sizeof(Vertex));
			vertexBufferResource.Map(std::move(posNormalList));

			indexBufferResource.Initialize(device, sizeof(decltype(faceList)::value_type) * faceList.size());
			indexBufferResource.Map(std::move(faceList));
		}

		void SetMesh(CommandList* cl)
		{
			cl->Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
			cl->Get()->IASetIndexBuffer(&indexBufferResource.GetView());
		}

		void Draw(CommandList* cl)
		{
			cl->Get()->DrawIndexedInstanced(faceNum * 3, 1, 0, 0, 0);
		}
	};

	template<std::size_t ColorObjectNum>
	class ColorObjectModel
	{
		std::array<ConstantBufferResource, ColorObjectNum> colorBunnyDataConstantBufferResource{};
		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneDataConstantBuffer, ConstantBufferResource* cubemapSceneDataConstantBuffer)
		{
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				colorBunnyDataConstantBufferResource[i].Initialize(device, sizeof(ColorObjectData));

			descriptorHeap.Initialize(device, 2 + ColorObjectNum);
			descriptorHeap.PushBackView(device, sceneDataConstantBuffer);
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				descriptorHeap.PushBackView(device, &colorBunnyDataConstantBufferResource[i]);
			descriptorHeap.PushBackView(device, cubemapSceneDataConstantBuffer);
		}

		template<typename T>
		void MapColorBunnyData(T&& t, std::size_t i)
		{
			colorBunnyDataConstantBufferResource[i].Map(std::forward<T>(t));
		}

		void SetDescriptorHeap(CommandList* cl, std::size_t i)
		{
			cl->Get()->SetDescriptorHeaps(1, &descriptorHeap.Get());
			cl->Get()->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle(0));
			cl->Get()->SetGraphicsRootDescriptorTable(1, descriptorHeap.GetGPUHandle(1 + i));
			cl->Get()->SetGraphicsRootDescriptorTable(2, descriptorHeap.GetGPUHandle(1 + ColorObjectNum));
		}
	};

	class ColorObjectRenderer
	{
		RootSignature rootSignature{};
		PipelineState standerdPipelineState{};
		PipelineState cubemapPipelineState{};

	public:
		void Initialize(Device* device)
		{
			rootSignature.Initialize(device,
				{ {DescriptorRangeType::CBV},{DescriptorRangeType::CBV }, { DescriptorRangeType::CBV } },
				{}
			);

			{
				Shader vs{};
				vs.Intialize(L"Shader/ColorObject/VertexShader.hlsl", "main", "vs_5_0");

				Shader ps{};
				ps.Intialize(L"Shader/ColorObject/PixelShader.hlsl", "main", "ps_5_0");

				standerdPipelineState.Initialize(device, &rootSignature, &vs, &ps,
					{ {"POSITION", VertexLayoutFormat::Float3 },{"NORMAL",VertexLayoutFormat::Float3 } }, 
					{ RenderTargetFormat::R8G8B8A8 }, true);
			}

			{
				Shader vs{};
				vs.Intialize(L"Shader/ColorObjectForCubemap/VertexShader.hlsl", "main", "vs_5_0");

				Shader gs{};
				gs.Intialize(L"Shader/ColorObjectForCubemap/GeometryShader.hlsl", "main", "gs_5_0");

				Shader ps{};
				ps.Intialize(L"Shader/ColorObjectForCubemap/PixelShader.hlsl", "main", "ps_5_0");

				cubemapPipelineState.Initialize(device, &rootSignature, &vs, &ps,
					{ {"POSITION", VertexLayoutFormat::Float3 } ,{"NORMAL",VertexLayoutFormat::Float3 } }, 
					{ RenderTargetFormat::R8G8B8A8 }, true, &gs);
			}
		}

		void SetStanderdPipelineState(CommandList* cl) {
			standerdPipelineState.PrepareForDrawing(cl);
		}

		void SetCubemapPipelineState(CommandList* cl) {
			cubemapPipelineState.PrepareForDrawing(cl);
		}
	};

	class MirrorObjectModel
	{
		ConstantBufferResource worldConstantbunnferResource{};
		CubeMapShaderResource cubemapShaderResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneConstantBufferResource)
		{
			worldConstantbunnferResource.Initialize(device, sizeof(XMMATRIX));
			cubemapShaderResource.Initialize(device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM , {0.5f,0.5f,0.5f,1.f} });

			descriptorHeap.Initialize(device, 3);
			descriptorHeap.PushBackView(device, sceneConstantBufferResource);
			descriptorHeap.PushBackView(device, &worldConstantbunnferResource);
			descriptorHeap.PushBackView(device, &cubemapShaderResource);
		}

		template<typename T>
		void MapWorld(T&& t) {
			worldConstantbunnferResource.Map(std::forward<T>(t));
		}

		auto& GetCubemapShaderResource() {
			return cubemapShaderResource;
		}

		void SetDescriptorHeap(CommandList* cl)
		{
			cl->Get()->SetDescriptorHeaps(1, &descriptorHeap.Get());
			cl->Get()->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
		}
	};


	class MirrorObjectRenderer
	{
		RootSignature rootSignature{};
		PipelineState pipelineState{};

	public:
		void Initialize(Device* device)
		{
			rootSignature.Initialize(device,
				{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV , DescriptorRangeType::SRV } },
				{ StaticSamplerType::Cubemap }
			);

			Shader vs{};
			vs.Intialize(L"Shader/MirrorObject/VertexShader.hlsl", "main", "vs_5_0");

			Shader ps{};
			ps.Intialize(L"Shader/MirrorObject/PixelShader.hlsl", "main", "ps_5_0");

			pipelineState.Initialize(device, &rootSignature, &vs, &ps,
				{ {"POSITION", VertexLayoutFormat::Float3 },{"NORMAL",VertexLayoutFormat::Float3 } },
				{ RenderTargetFormat::R8G8B8A8 }, true);
		}

		void SetPipelineState(CommandList* cl) {
			pipelineState.PrepareForDrawing(cl);
		}
	};


	inline CubemapSceneData GetCubemapSceneData(const XMFLOAT3& pos)
	{
		CubemapSceneData result;

		std::array<XMFLOAT3, 6> up{
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,0.f,-1.f),
			XMFLOAT3(0.f,0.f,1.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
		};
		std::array<XMFLOAT3, 6> target{
			XMFLOAT3(1.f,0.f,0.f),
			XMFLOAT3(-1.f,0.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,-1.f,0.f),
			XMFLOAT3(0.f,0.f,1.f),
			XMFLOAT3(0.f,0.f,-1.f),
		};

		for (std::size_t i = 0; i < 6; i++)
			result.view[i] = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target[i]), XMLoadFloat3(&up[i]));

		result.proj = XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2,
			static_cast<float>(CUBE_MAP_EDGE) / static_cast<float>(CUBE_MAP_EDGE), 0.1f, 100.f);

		return result;
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


		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));

		ConstantBufferResource cubemapSceneDataConstantBuffer{};
		cubemapSceneDataConstantBuffer.Initialize(&device, sizeof(CubemapSceneData));


		Mesh bunnyMesh{};
		bunnyMesh.Initialize(&device, "../../Assets/bun_zipper_res3.off");

		Mesh sphereMesh{};
		sphereMesh.Initialize(&device, "../../Assets/sphere.off");


		constexpr std::size_t ColorObjectNum = 4;

		std::array<XMMATRIX, ColorObjectNum> colorObjectWorlds{
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(10.f,0.f,0.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(-9.f,0.f,0.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(0.f, 0.f, 7.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(0.f, 0.f, -6.f)
		};

		std::array<std::array<float, 4>, ColorObjectNum> colorObjectColors{ {
			{1.f, 0.f, 0.f, 1.f},
			{0.f, 1.f, 0.f, 1.f},
			{0.f, 0.f, 1.f, 1.f},
			{1.f, 1.f, 0.f, 1.f}
		} };

		ColorObjectModel<ColorObjectNum> colorObjectModel{};
		colorObjectModel.Initialize(&device, &sceneDataConstantBufferResource, &cubemapSceneDataConstantBuffer);

		ColorObjectRenderer colorObjectRenderer{};
		colorObjectRenderer.Initialize(&device);


		MirrorObjectModel mirrorObjectModel{};
		mirrorObjectModel.Initialize(&device, &sceneDataConstantBufferResource);

		MirrorObjectRenderer mirrorObjectRenderer{};
		mirrorObjectRenderer.Initialize(&device);


		CubeMapDepthStencilBufferResource cubemapDepthStencilBufferResource{};
		cubemapDepthStencilBufferResource.Initialize(&device, CUBE_MAP_EDGE, CUBE_MAP_EDGE);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> cubemapRtvDescriptorHeap{};
		cubemapRtvDescriptorHeap.Initialize(&device, 1);
		cubemapRtvDescriptorHeap.PushBackView(&device, &mirrorObjectModel.GetCubemapShaderResource());


		DepthStencilBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 2);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);
		//Ç±Ç±Ç…ViewÇçÏÇ¡ÇƒÇµÇ‹Ç®Ç§
		depthStencilDescriptorHeap.PushBackView(&device, &cubemapDepthStencilBufferResource);



		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT cubemapViewport{ 0,0, static_cast<float>(CUBE_MAP_EDGE),static_cast<float>(CUBE_MAP_EDGE),0.f,1.f };
		D3D12_RECT cubemapScissorRect{ 0,0,static_cast<LONG>(CUBE_MAP_EDGE),static_cast<LONG>(CUBE_MAP_EDGE) };

		float len = 4.f;
		XMFLOAT3 eye{ len,5,len };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			100.f
		);
		XMFLOAT4 lightDir{ 1.f,1.f,1.f,1.f };
		XMFLOAT3 mirrorPos{ 0.f, 0.f, 0.f };

		while (UpdateWindow())
		{

			sceneDataConstantBufferResource.Map(SceneData{ view,proj,lightDir,eye });

			for (std::size_t i = 0; i < ColorObjectNum; i++) {
				colorObjectWorlds[i] *= XMMatrixRotationY(0.02);
				colorObjectModel.MapColorBunnyData(ColorObjectData{ colorObjectWorlds[i] ,colorObjectColors[i] }, i);
			}

			auto cubemapSceneData = GetCubemapSceneData(mirrorPos);
			cubemapSceneDataConstantBuffer.Map(cubemapSceneData);

			mirrorObjectModel.MapWorld(XMMatrixScaling(3.f, 3.f, 3.f) * XMMatrixTranslation(mirrorPos.x, mirrorPos.y, mirrorPos.z));


			//Cubemap
			{
				commandList.SetViewport(cubemapViewport);
				commandList.SetScissorRect(cubemapScissorRect);

				mirrorObjectModel.GetCubemapShaderResource().Barrior(&commandList, ResourceState::RenderTarget);
				commandList.Get()->ClearRenderTargetView(cubemapRtvDescriptorHeap.GetCPUHandle(),
					mirrorObjectModel.GetCubemapShaderResource().GetClearValue()->Color, 0, nullptr);

				commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(1),
					D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

				commandList.SetRenderTarget(cubemapRtvDescriptorHeap.GetCPUHandle(), depthStencilDescriptorHeap.GetCPUHandle(1));

				colorObjectRenderer.SetCubemapPipelineState(&commandList);
				bunnyMesh.SetMesh(&commandList);

				for (std::size_t i = 0; i < ColorObjectNum; i++) {
					colorObjectModel.SetDescriptorHeap(&commandList, i);
					bunnyMesh.Draw(&commandList);
				}

				mirrorObjectModel.GetCubemapShaderResource().Barrior(&commandList, ResourceState::PixcelShaderResource);
			}

			//BackBuffer
			{
				commandList.SetViewport(viewport);
				commandList.SetScissorRect(scissorRect);

				doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
				doubleBuffer.ClearBackBuffer(&commandList);

				commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(),
					D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

				commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(), depthStencilDescriptorHeap.GetCPUHandle());



				//ColorBunny
				{
					colorObjectRenderer.SetStanderdPipelineState(&commandList);
					bunnyMesh.SetMesh(&commandList);

					for (std::size_t i = 0; i < ColorObjectNum; i++) {
						colorObjectModel.SetDescriptorHeap(&commandList, i);
						bunnyMesh.Draw(&commandList);
					}
				}

				//MirrorSphere
				{
					mirrorObjectRenderer.SetPipelineState(&commandList);
					mirrorObjectModel.SetDescriptorHeap(&commandList);
					sphereMesh.SetMesh(&commandList);
					sphereMesh.Draw(&commandList);
				}

				doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);
			}


			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		}

		return 0;
	}



}