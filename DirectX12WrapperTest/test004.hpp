#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"CommandList.hpp"
#include"DoubleBuffer.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState/PipelineState.hpp"
#include"Resource/IndexBufferResource.hpp"
#include"Resource/ShaderResource.hpp"
#include"Resource/ConstantBufferResource.hpp"

#include"OffLoader.hpp"

#include<array>
#include<DirectXMath.h>

//動的な環境マップを扱う
namespace test004
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

	using World = XMMATRIX;
	using Color = XMFLOAT4;

	struct ColorBunnyData {
		World world;
		Color colot;
	};

	constexpr std::size_t CUBE_MAP_EDGE = 512;

	struct CubemapSceneData {
		XMMATRIX view[6];
		XMMATRIX proj;
	};


	class BunnyMesh
	{
		std::size_t faceNum{};

		VertexBufferResource vertexBufferResource{};
		IndexBufferResource indexBufferResource{};

	public:
		void Initialize(Device* device)
		{
			auto [bunnyVertex, bunnyFace] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("../../Assets/bun_zipper.off");

			faceNum = bunnyFace.size();

			vertexBufferResource.Initialize(device, sizeof(decltype(bunnyVertex)::value_type) * bunnyVertex.size(), sizeof(decltype(bunnyVertex)::value_type));
			vertexBufferResource.Map(std::move(bunnyVertex));

			indexBufferResource.Initialize(device, sizeof(decltype(bunnyFace)::value_type) * bunnyFace.size());
			indexBufferResource.Map(std::move(bunnyFace));
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

	template<std::size_t ColorBunnyNum>
	class ColorBunnyModel
	{
		std::array<ConstantBufferResource, ColorBunnyNum> colorBunnyDataConstantBufferResource{};
		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneDataConstantBuffer, ConstantBufferResource* cubemapSceneDataConstantBuffer)
		{
			for (std::size_t i = 0; i < ColorBunnyNum; i++)
				colorBunnyDataConstantBufferResource[i].Initialize(device, sizeof(ColorBunnyData));

			descriptorHeap.Initialize(device, 2 + ColorBunnyNum);
			descriptorHeap.PushBackView(device, sceneDataConstantBuffer);
			for (std::size_t i = 0; i < ColorBunnyNum; i++)
				descriptorHeap.PushBackView(device, &colorBunnyDataConstantBufferResource[i]);
			descriptorHeap.PushBackView(device, cubemapSceneDataConstantBuffer);
		}

		template<typename T>
		void MapColorBunnyData( T&& t, std::size_t i) 
		{
			colorBunnyDataConstantBufferResource[i].Map(std::forward<T>(t));
		}

		void SetDescriptorHeap(CommandList* cl, std::size_t i) 
		{
			cl->Get()->SetDescriptorHeaps(1, &descriptorHeap.Get());
			cl->Get()->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle(0));
			cl->Get()->SetGraphicsRootDescriptorTable(1, descriptorHeap.GetGPUHandle(1 + i));
			cl->Get()->SetGraphicsRootDescriptorTable(2, descriptorHeap.GetGPUHandle(1 + ColorBunnyNum));
		}
	};

	class ColorBunnyRenderer
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
				vs.Intialize(L"Shader/ColorBunny/VertexShader.hlsl", "main", "vs_5_0");

				Shader gs{};
				gs.Intialize(L"Shader/ColorBunny/GeometryShader.hlsl", "main", "gs_5_0");

				Shader ps{};
				ps.Intialize(L"Shader/ColorBunny/PixelShader.hlsl", "main", "ps_5_0");

				standerdPipelineState.Initialize(device, &rootSignature, &vs, &ps,
					{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &gs);
			}

			{
				Shader vs{};
				vs.Intialize(L"Shader/ColorBunnyForCubemap/VertexShader.hlsl", "main", "vs_5_0");

				Shader gs{};
				gs.Intialize(L"Shader/ColorBunnyForCubemap/GeometryShader.hlsl", "main", "gs_5_0");

				Shader ps{};
				ps.Intialize(L"Shader/ColorBunnyForCubemap/PixelShader.hlsl", "main", "ps_5_0");

				cubemapPipelineState.Initialize(device, &rootSignature, &vs, &ps,
					{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &gs);
			}
		}

		void SetStanderdPipelineState(CommandList* cl) {
			standerdPipelineState.PrepareForDrawing(cl);
		}

		void SetCubemapPipelineState(CommandList* cl) {
			cubemapPipelineState.PrepareForDrawing(cl);
		}
	};

	class MirrorBunnyModel
	{
		ConstantBufferResource worldConstantbunnferResource{};
		CubeMapShaderResource cubemapShaderResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneConstantBufferResource)
		{
			worldConstantbunnferResource.Initialize(device, sizeof(World));
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


	class MirrorBunnyRenderer
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
			vs.Intialize(L"Shader/MirrorBunny/VertexShader.hlsl", "main", "vs_5_0");

			Shader gs{};
			gs.Intialize(L"Shader/MirrorBunny/GeometryShader.hlsl", "main", "gs_5_0");

			Shader ps{};
			ps.Intialize(L"Shader/MirrorBunny/PixelShader.hlsl", "main", "ps_5_0");

			pipelineState.Initialize(device, &rootSignature, &vs, &ps,
				{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &gs);
		}

		void SetPipelineState(CommandList* cl){
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


		constexpr std::size_t ColorBunnyNum = 4;

		std::array<World, ColorBunnyNum> colorBunnyWorlds{
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(5.f,0.f,0.f),
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(-4.f,0.f,0.f),
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f, 0.f, 3.f),
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f, 0.f, -2.f)
		};
		std::array<Color, ColorBunnyNum> colorBunnyColors{
			XMFLOAT4(1.f, 0.f, 0.f, 1.f),
			XMFLOAT4(0.f, 1.f, 0.f, 1.f),
			XMFLOAT4(0.f, 0.f, 1.f, 1.f),
			XMFLOAT4(1.f, 1.f, 0.f, 1.f)
		};



		BunnyMesh bunnyMesh{};
		bunnyMesh.Initialize(&device);

		ColorBunnyModel<ColorBunnyNum> colorBunnyModel{};
		colorBunnyModel.Initialize(&device, &sceneDataConstantBufferResource, &cubemapSceneDataConstantBuffer);

		ColorBunnyRenderer colorBunnyRenderer{};
		colorBunnyRenderer.Initialize(&device);

		MirrorBunnyModel mirrorBunnyModel{};
		mirrorBunnyModel.Initialize(&device, &sceneDataConstantBufferResource);
		mirrorBunnyModel.MapWorld(XMMatrixScaling(20.f, 20.f, 20.f));

		MirrorBunnyRenderer mirrorBunnyRenderer{};
		mirrorBunnyRenderer.Initialize(&device);


		CubeMapDepthStencilBufferResource cubemapDepthStencilBufferResource{};
		cubemapDepthStencilBufferResource.Initialize(&device, CUBE_MAP_EDGE, CUBE_MAP_EDGE);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> cubemapRtvDescriptorHeap{};
		cubemapRtvDescriptorHeap.Initialize(&device, 1);
		cubemapRtvDescriptorHeap.PushBackView(&device, &mirrorBunnyModel.GetCubemapShaderResource());


		DepthStencilBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 2);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);
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

		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			eye.x = len * std::cos(cnt / 100.0);
			eye.z = len * std::sin(cnt / 100.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			cnt++;


			sceneDataConstantBufferResource.Map(SceneData{ view,proj,lightDir,eye });

			auto cubemapSceneData = GetCubemapSceneData(XMFLOAT3(0.f, 0.f, 0.f));
			cubemapSceneDataConstantBuffer.Map(cubemapSceneData);

			for (std::size_t i = 0; i < ColorBunnyNum; i++) {
				colorBunnyWorlds[i] *= XMMatrixRotationY(0.02);
				colorBunnyModel.MapColorBunnyData(ColorBunnyData{ colorBunnyWorlds[i] ,colorBunnyColors[i] }, i);
			}


			//Cubemap
			{
				commandList.SetViewport(cubemapViewport);
				commandList.SetScissorRect(cubemapScissorRect);

				mirrorBunnyModel.GetCubemapShaderResource().Barrior(&commandList, ResourceState::RenderTarget);
				commandList.Get()->ClearRenderTargetView(cubemapRtvDescriptorHeap.GetCPUHandle(),
					mirrorBunnyModel.GetCubemapShaderResource().GetClearValue()->Color, 0, nullptr);

				commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(1),
					D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

				commandList.SetRenderTarget(cubemapRtvDescriptorHeap.GetCPUHandle(), depthStencilDescriptorHeap.GetCPUHandle(1));

				colorBunnyRenderer.SetCubemapPipelineState(&commandList);
				bunnyMesh.SetMesh(&commandList);

				for (std::size_t i = 0; i < ColorBunnyNum; i++) {
					colorBunnyModel.SetDescriptorHeap(&commandList, i);
					bunnyMesh.Draw(&commandList);
				}

				mirrorBunnyModel.GetCubemapShaderResource().Barrior(&commandList, ResourceState::PixcelShaderResource);
			}
			
			
			//バックバッファへの描写
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
					colorBunnyRenderer.SetStanderdPipelineState(&commandList);
					bunnyMesh.SetMesh(&commandList);

					for (std::size_t i = 0; i < ColorBunnyNum; i++) {
						colorBunnyModel.SetDescriptorHeap(&commandList, i);
						bunnyMesh.Draw(&commandList);
					}
				}

				//MirrorBunny
				{
					mirrorBunnyRenderer.SetPipelineState(&commandList);
					mirrorBunnyModel.SetDescriptorHeap(&commandList);
					bunnyMesh.SetMesh(&commandList);
					bunnyMesh.Draw(&commandList);
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