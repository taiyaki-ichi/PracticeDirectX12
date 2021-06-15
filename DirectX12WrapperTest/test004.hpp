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

	struct Vertex {
		float x, y, z;
		float u, v;
	};

	struct SceneData {
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 lightDir;
	};

	using World = XMMATRIX;
	using Color = XMFLOAT4;

	constexpr std::size_t CUBE_MAP_SIZE = 512;

	class ColorBunnyResources 
	{
		ConstantBufferResource worldConstantbunnferResource{};
		ConstantBufferResource colorConstantBufferResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBufferResource* sceneDataConstantBuffer) 
		{
			worldConstantbunnferResource.Initialize(device, sizeof(World));
			colorConstantBufferResource.Initialize(device, sizeof(Color));

			descriptorHeap.Initialize(device, 3);
			descriptorHeap.PushBackView(device, &worldConstantbunnferResource);
			descriptorHeap.PushBackView(device, &colorConstantBufferResource);
			descriptorHeap.PushBackView(device, sceneDataConstantBuffer);
		}

		void MapWorld(const World& w) {
			worldConstantbunnferResource.Map(w);
		}

		void MapColor(const Color& c) {
			colorConstantBufferResource.Map(c);
		}

		auto& GetDecriptorHeap() {
			return descriptorHeap;
		}
	};

	inline int main()
	{
		
		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

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


		auto [bunnyVertex, bunnyFace] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint16_t, 3>>("Assets/bun_zipper.off");

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(decltype(bunnyVertex)::value_type) * bunnyVertex.size(), sizeof(decltype(bunnyVertex)::value_type));
		vertexBufferResource.Map(bunnyVertex);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(decltype(bunnyFace)::value_type) * bunnyFace.size());
		indexBufferResource.Map(bunnyFace);


		Shader colorBunnyVertexShader{};
		colorBunnyVertexShader.Intialize(L"Shader/VertexShader004_ColorBunny.hlsl", "main", "vs_5_0");

		Shader colorBunnyPixelShader{};
		colorBunnyPixelShader.Intialize(L"Shader/PixelShader004_ColorBunny.hlsl", "main", "ps_5_0");

		Shader colorBunnyGeometryShader{};
		colorBunnyGeometryShader.Intialize(L"Shader/GeometryShader004_ColorBunny.hlsl", "main", "gs_5_0");

		RootSignature colorBunnyRootSignature{};
		colorBunnyRootSignature.Initialize(&device, { {DescriptorRangeType::CBV,DescriptorRangeType::CBV,DescriptorRangeType::CBV,} }, {});

		PipelineState colorBunnyPipelineState{};
		colorBunnyPipelineState.Initialize(&device, &colorBunnyRootSignature, &colorBunnyVertexShader, &colorBunnyPixelShader,
			{ {"POSITION", VertexLayoutFormat::Float3 } }, { RenderTargetFormat::R8G8B8A8 }, true, &colorBunnyGeometryShader);

		constexpr std::size_t ColorBunnyNum = 3;
		std::array<World, ColorBunnyNum> colorBunnyWorlds{
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f,0.f,0.f),
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(0.f,0.f,3.f),
			XMMatrixScaling(10.f, 10.f, 10.f) * XMMatrixTranslation(3.f, 0.f, 3.f)
		};
		std::array<Color, ColorBunnyNum> colorBunnyColors{
			XMFLOAT4(1.f, 0.f, 0.f, 1.f),
			XMFLOAT4(0.f, 1.f, 0.f, 1.f),
			XMFLOAT4(0.f, 0.f, 1.f, 1.f)
		};

		std::array<ColorBunnyResources, ColorBunnyNum> colorBunnyResources{};
		for (std::size_t i = 0; i < ColorBunnyNum; i++) {
			colorBunnyResources[i].Initialize(&device, &sceneDataConstantBufferResource);
			colorBunnyResources[i].MapWorld(colorBunnyWorlds[i]);
			colorBunnyResources[i].MapColor(colorBunnyColors[i]);
		}


		DepthStencilBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);

		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };



		XMFLOAT3 eye{ 0,3,3 };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			100.f
		);
		XMFLOAT4 lightDir{ 1.f,1.f,1.f,1.f };

		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			eye.x = 3 * std::cos(cnt / 60.0);
			eye.z = 3 * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			cnt++;

			sceneDataConstantBufferResource.Map(SceneData{ view,proj,lightDir });

			commandList.SetViewport(viewport);
			commandList.SetScissorRect(scissorRect);

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::RenderTarget);
			doubleBuffer.ClearBackBuffer(&commandList);

			commandList.Get()->ClearDepthStencilView(depthStencilDescriptorHeap.GetCPUHandle(),
				D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

			commandList.SetRenderTarget(doubleBuffer.GetBackbufferCpuHandle(), depthStencilDescriptorHeap.GetCPUHandle());

			colorBunnyPipelineState.PrepareForDrawing(&commandList);

			commandList.Get()->IASetVertexBuffers(0, 1, &vertexBufferResource.GetView());
			commandList.Get()->IASetIndexBuffer(&indexBufferResource.GetView());

			for (std::size_t i = 0; i < ColorBunnyNum; i++)
			{
				commandList.Get()->SetDescriptorHeaps(1, &colorBunnyResources[i].GetDecriptorHeap().Get());
				commandList.Get()->SetGraphicsRootDescriptorTable(0, colorBunnyResources[i].GetDecriptorHeap().GetGPUHandle());

				commandList.Get()->DrawIndexedInstanced(bunnyFace.size() * 3, 1, 0, 0, 0);
			}

			doubleBuffer.BarriorToBackbuffer(&commandList, ResourceState::Common);

			commandList.Close();
			commandList.Execute();
			commandList.Clear();

			doubleBuffer.Flip();
		}


		return 0;
	}
}