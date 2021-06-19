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

//テクスチャの表示をする
namespace test004
{
	using namespace DX12;
	using namespace DirectX;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct Vertex {
		float x, y, z;
		float u, v;
	};

	struct SceneData {
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 lightDir;
		XMFLOAT3 eye;
	};

	using World = XMMATRIX;
	using Color = XMFLOAT4;

	constexpr std::size_t CUBE_MAP_EDGE = 512;

	struct CubemapSceneData {
		XMMATRIX view[6];
		XMMATRIX proj;
	};

	class ColorBunnyResources 
	{
		ConstantBufferResource worldConstantbunnferResource{};
		ConstantBufferResource colorConstantBufferResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneDataConstantBuffer, ConstantBufferResource* cubemapSceneDataConstantBuffer)
		{
			worldConstantbunnferResource.Initialize(device, sizeof(World));
			colorConstantBufferResource.Initialize(device, sizeof(Color));

			descriptorHeap.Initialize(device, 4);
			descriptorHeap.PushBackView(device, sceneDataConstantBuffer);
			descriptorHeap.PushBackView(device, &worldConstantbunnferResource);
			descriptorHeap.PushBackView(device, &colorConstantBufferResource);
			descriptorHeap.PushBackView(device, cubemapSceneDataConstantBuffer);
		}

		void MapWorld(const World& w) {
			worldConstantbunnferResource.Map(w);
		}

		void MapColor(const Color& c) {
			colorConstantBufferResource.Map(c);
		}

		auto& GetDescriptorHeap() {
			return descriptorHeap;
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

	class MirrorBunnyResources
	{
		ConstantBufferResource worldConstantbunnferResource{};
		CubeMapShaderResource cubemapShaderResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> cbvSrvDescriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneConstantBufferResource) {
			worldConstantbunnferResource.Initialize(device, sizeof(World));
			cubemapShaderResource.Initialize(device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM , {0.5f,0.5f,0.5f,1.f} });

			cbvSrvDescriptorHeap.Initialize(device, 3);
			cbvSrvDescriptorHeap.PushBackView(device, sceneConstantBufferResource);
			cbvSrvDescriptorHeap.PushBackView(device, &worldConstantbunnferResource);
			cbvSrvDescriptorHeap.PushBackView(device, &cubemapShaderResource);
		}

		void MapWorld(const World& w) {
			worldConstantbunnferResource.Map(w);
		}

		auto& GetCubemapShaderResource() {
			return cubemapShaderResource;
		}

		auto& GetDescriptorHeap() {
			return cbvSrvDescriptorHeap;
		}
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


		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));

		
		std::array<std::array<float, 3>, 4> mirrorVertex{ {
			{0.05f,-0.1f,-0.05f},
			{0.05f,0.1f,-0.05f},
			{-0.05f,-0.1f,0.05f},
			{-0.05f,0.1f,0.05f}
			} };
			

		std::array<std::uint16_t, 6> mirrorIndex{
			0,1,2,2,1,3
		};

		VertexBufferResource mirrorVertexBufferResource{};
		mirrorVertexBufferResource.Initialize(&device, sizeof(decltype(mirrorVertex)::value_type) * mirrorVertex.size(), sizeof(decltype(mirrorVertex)::value_type));
		mirrorVertexBufferResource.Map(mirrorVertex);

		IndexBufferResource mirrorIndexBufferResource{};
		mirrorIndexBufferResource.Initialize(&device, sizeof(decltype(mirrorIndex)::value_type) * mirrorIndex.size());
		mirrorIndexBufferResource.Map(mirrorIndex);


		auto [bunnyVertex, bunnyFace] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("../../Assets/bun_zipper.off");

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(decltype(bunnyVertex)::value_type) * bunnyVertex.size(), sizeof(decltype(bunnyVertex)::value_type));
		vertexBufferResource.Map(bunnyVertex);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(decltype(bunnyFace)::value_type) * bunnyFace.size());
		indexBufferResource.Map(bunnyFace);


		ConstantBufferResource cubemapSceneDataConstantBuffer{};
		cubemapSceneDataConstantBuffer.Initialize(&device, sizeof(CubemapSceneData));


		Shader colorBunnyVertexShader{};
		colorBunnyVertexShader.Intialize(L"Shader/VertexShader004_ColorBunny.hlsl", "main", "vs_5_0");

		Shader colorBunnyPixelShader{};
		colorBunnyPixelShader.Intialize(L"Shader/PixelShader004_ColorBunny.hlsl", "main", "ps_5_0");

		Shader colorBunnyGeometryShader{};
		colorBunnyGeometryShader.Intialize(L"Shader/GeometryShader004_ColorBunny.hlsl", "main", "gs_5_0");

		RootSignature colorBunnyRootSignature{};
		colorBunnyRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::CBV} },
			{});

		PipelineState colorBunnyPipelineState{};
		colorBunnyPipelineState.Initialize(&device, &colorBunnyRootSignature, &colorBunnyVertexShader, &colorBunnyPixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &colorBunnyGeometryShader);

		Shader pixelShaderForCubamap{};
		pixelShaderForCubamap.Intialize(L"Shader/PixelShader004_ColorBunnyForCubemap.hlsl", "main", "ps_5_0");

		Shader geometryShaderForCubemap{};
		geometryShaderForCubemap.Intialize(L"Shader/GeometryShader004_ColorBunnyForCubemap.hlsl", "main", "gs_5_0");

		PipelineState colorBunnyPipelineStateForCubemap{};
		colorBunnyPipelineStateForCubemap.Initialize(&device, &colorBunnyRootSignature, &colorBunnyVertexShader, &pixelShaderForCubamap,
			{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &geometryShaderForCubemap);


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

		std::array<ColorBunnyResources, ColorBunnyNum> colorBunnyResources{};
		for (std::size_t i = 0; i < ColorBunnyNum; i++) {
			colorBunnyResources[i].Initialize(&device, &sceneDataConstantBufferResource, &cubemapSceneDataConstantBuffer);
			colorBunnyResources[i].MapWorld(colorBunnyWorlds[i]);
			colorBunnyResources[i].MapColor(colorBunnyColors[i]);
		}


		Shader mirrorBunnyPixelShader{};
		mirrorBunnyPixelShader.Intialize(L"Shader/PixelShader004_MirrorBunny.hlsl", "main", "ps_5_0");

		RootSignature mirrorBunnyRootSignature{};
		mirrorBunnyRootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::Cubemap });

		PipelineState mirrorBunnyPipelineState{};
		mirrorBunnyPipelineState.Initialize(&device, &mirrorBunnyRootSignature, &colorBunnyVertexShader, &mirrorBunnyPixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &colorBunnyGeometryShader);


		MirrorBunnyResources mirrorBunnyResources{};
		mirrorBunnyResources.Initialize(&device, &sceneDataConstantBufferResource);
		mirrorBunnyResources.MapWorld(XMMatrixScaling(20.f, 20.f, 20.f));


		CubeMapDepthStencilBufferResource cubemapDepthStencilBufferResource{};
		cubemapDepthStencilBufferResource.Initialize(&device, CUBE_MAP_EDGE, CUBE_MAP_EDGE);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> cubemapRtvDescriptorHeap{};
		cubemapRtvDescriptorHeap.Initialize(&device, 1);
		cubemapRtvDescriptorHeap.PushBackView(&device, &mirrorBunnyResources.GetCubemapShaderResource());


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

			for (std::size_t i = 0; i < colorBunnyWorlds.size(); i++) {
				colorBunnyWorlds[i] *= XMMatrixRotationY(0.02);
				colorBunnyResources[i].MapWorld(colorBunnyWorlds[i]);
			}

			sceneDataConstantBufferResource.Map(SceneData{ view,proj,lightDir,eye });

			auto cubemapSceneData = GetCubemapSceneData(XMFLOAT3(0.f, 0.f, 0.f));
			cubemapSceneDataConstantBuffer.Map(cubemapSceneData);


			//cubemap作成
			{
				commandList.SetViewport(cubemapViewport);
				commandList.SetScissorRect(cubemapScissorRect);

				mirrorBunnyResources.GetCubemapShaderResource().Barrior(&commandList, ResourceState::RenderTarget);
				commandList.Get()->ClearRenderTargetView(cubemapRtvDescriptorHeap.GetCPUHandle(),
					mirrorBunnyResources.GetCubemapShaderResource().GetClearValue()->Color, 0, nullptr);

				commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(1),
					D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

				commandList.SetRenderTarget(cubemapRtvDescriptorHeap.GetCPUHandle(), depthStencilDescriptorHeap.GetCPUHandle(1));

				colorBunnyPipelineStateForCubemap.PrepareForDrawing(&commandList);

				commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
				commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());

				for (std::size_t i = 0; i < ColorBunnyNum; i++)
				{
					commandList.Get()->SetDescriptorHeaps(1, &colorBunnyResources[i].GetDescriptorHeap().Get());
					commandList.Get()->SetGraphicsRootDescriptorTable(0, colorBunnyResources[i].GetDescriptorHeap().GetGPUHandle());

					commandList.Get()->DrawIndexedInstanced(bunnyFace.size() * 3, 1, 0, 0, 0);
				}

				mirrorBunnyResources.GetCubemapShaderResource().Barrior(&commandList, ResourceState::PixcelShaderResource);
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

				//colorBunny
				{
					colorBunnyPipelineState.PrepareForDrawing(&commandList);

					commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
					commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());

					for (std::size_t i = 0; i < ColorBunnyNum; i++)
					{
						commandList.Get()->SetDescriptorHeaps(1, &colorBunnyResources[i].GetDescriptorHeap().Get());
						commandList.Get()->SetGraphicsRootDescriptorTable(0, colorBunnyResources[i].GetDescriptorHeap().GetGPUHandle());

						commandList.Get()->DrawIndexedInstanced(bunnyFace.size() * 3, 1, 0, 0, 0);
					}
				}

				//mirrorBunny
				{
					mirrorBunnyPipelineState.PrepareForDrawing(&commandList);

					commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
					commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());
					//commandList.Get()->IASetVertexBuffers(0, 1, &mirrorVertexBufferResource.GetView());
					//commandList.Get()->IASetIndexBuffer(&mirrorIndexBufferResource.GetView());

					commandList.Get()->SetDescriptorHeaps(1, &mirrorBunnyResources.GetDescriptorHeap().Get());
					commandList.Get()->SetGraphicsRootDescriptorTable(0, mirrorBunnyResources.GetDescriptorHeap().GetGPUHandle());

					commandList.Get()->DrawIndexedInstanced(bunnyFace.size() * 3, 1, 0, 0, 0);
					//commandList.Get()->DrawIndexedInstanced(mirrorIndex.size(), 1, 0, 0, 0);

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