#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"PipelineState.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"
#include"Resource/VertexBuffer.hpp"
#include"Resource/IndexBuffer.hpp"
#include"Resource/ShaderResource.hpp"
#include"Resource/ConstantBuffer.hpp"
#include"Resource/DepthBuffer.hpp"

#include<array>
#include<DirectXMath.h>


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

	struct ColorObjectData {
		XMMATRIX world;
		std::array<float, 4> color;
	};

	constexpr std::size_t CUBE_MAP_EDGE = 512;

	struct CubemapSceneData {
		XMMATRIX view[6];
		XMMATRIX proj;
	};

	constexpr std::size_t FRAME_LATENCY_NUM = 2;

	class Mesh
	{
		std::size_t faceNum{};

		VertexBuffer vertexBuffer{};
		IndexBuffer indexBuffer{};

		struct Vertex {
			std::array<float, 3> pos;
			std::array<float, 3> normal;
		};

	public:
		void Initialize(Device* device,const char* fileName)
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>(fileName);
			auto normalList = GetVertexNormal(vertexList, faceList);

			std::vector<Vertex> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			faceNum = faceList.size();

			vertexBuffer.Initialize(device, posNormalList.size(), sizeof(Vertex));
			vertexBuffer.Map(std::move(posNormalList));

			indexBuffer.Initialize(device, faceList.size() * 3);
			indexBuffer.Map(std::move(faceList));
		}

		void SetMesh(Command<FRAME_LATENCY_NUM>* cl)
		{
			cl->SetVertexBuffer(&vertexBuffer);
			cl->SetIndexBuffer(&indexBuffer);
		}

		void Draw(Command<FRAME_LATENCY_NUM>* cl)
		{
			cl->DrawIndexedInstanced(faceNum * 3);
		}
	};

	template<std::size_t ColorObjectNum>
	class ColorObjectModel
	{
		std::array<ConstantBuffer, ColorObjectNum> colorBunnyDataConstantBuffer{};
		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		void Initialize(Device* device, ConstantBuffer* sceneDataConstantBuffer, ConstantBuffer* cubemapSceneDataConstantBuffer)
		{
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				colorBunnyDataConstantBuffer[i].Initialize(device, sizeof(ColorObjectData));

			descriptorHeap.Initialize(device, 2 + ColorObjectNum);
			descriptorHeap.PushBackView(device, sceneDataConstantBuffer);
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				descriptorHeap.PushBackView(device, &colorBunnyDataConstantBuffer[i]);
			descriptorHeap.PushBackView(device, cubemapSceneDataConstantBuffer);
		}

		template<typename T>
		void MapColorBunnyData(T&& t, std::size_t i)
		{
			colorBunnyDataConstantBuffer[i].Map(std::forward<T>(t));
		}

		void SetDescriptorHeap(Command<FRAME_LATENCY_NUM>* cl, std::size_t i)
		{
			cl->SetDescriptorHeap(&descriptorHeap);
			cl->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle(0));
			cl->SetGraphicsRootDescriptorTable(1, descriptorHeap.GetGPUHandle(1 + i));
			cl->SetGraphicsRootDescriptorTable(2, descriptorHeap.GetGPUHandle(1 + ColorObjectNum));
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

				standerdPipelineState.Initialize(device, &rootSignature, { &vs, &ps },
					{ {"POSITION", {Type::Float,3} },{"NORMAL",{Type::Float,3} } },
					{ {Type::UnsignedNormalizedFloat,4} }, true, false, PrimitiveTopology::Triangle
				);
			}

			{
				Shader vs{};
				vs.Intialize(L"Shader/ColorObjectForCubemap/VertexShader.hlsl", "main", "vs_5_0");

				Shader gs{};
				gs.Intialize(L"Shader/ColorObjectForCubemap/GeometryShader.hlsl", "main", "gs_5_0");

				Shader ps{};
				ps.Intialize(L"Shader/ColorObjectForCubemap/PixelShader.hlsl", "main", "ps_5_0");

				cubemapPipelineState.Initialize(device, &rootSignature, { &vs, &ps,&gs },
					{ {"POSITION", {Type::Float,3} } ,{"NORMAL",{Type::Float,3} } },
					{ {Type::UnsignedNormalizedFloat,4} }, true, false, PrimitiveTopology::Triangle
				);
			}
		}

		PipelineState& GetStanderdPipelineState() noexcept {
			return standerdPipelineState;
		}
		PipelineState& GetCubemapPipelineState() noexcept {
			return cubemapPipelineState;
		}
		RootSignature& GetRootSignature() noexcept {
			return rootSignature;
		}

	};

	class MirrorObjectModel
	{
		ConstantBuffer worldConstantBuffer{};
		ShaderResource cubemapShaderResource{};

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};

	public:
		static constexpr std::array<float, 4> CUBEMAP_CLEAR_VALUE{ 0.5f,0.5f,0.5f,1.f };

		void Initialize(Device* device, ConstantBuffer* sceneConstantBufferResource)
		{
			worldConstantBuffer.Initialize(device, sizeof(XMMATRIX));
			cubemapShaderResource.Initialize(device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, { Type::UnsignedNormalizedFloat,4 }, 6, CUBEMAP_CLEAR_VALUE);

			descriptorHeap.Initialize(device, 3);
			descriptorHeap.PushBackView(device, sceneConstantBufferResource);
			descriptorHeap.PushBackView(device, &worldConstantBuffer);
			descriptorHeap.PushBackView<ViewTypeTag::CubeMap>(device, &cubemapShaderResource);
		}

		template<typename T>
		void MapWorld(T&& t) {
			worldConstantBuffer.Map(std::forward<T>(t));
		}

		auto& GetCubemapShaderResource() {
			return cubemapShaderResource;
		}

		void SetDescriptorHeap(Command<FRAME_LATENCY_NUM>* cl)
		{
			cl->SetDescriptorHeap(&descriptorHeap);
			cl->SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
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

			pipelineState.Initialize(device, &rootSignature, { &vs, &ps },
				{ {"POSITION", {Type::Float,3} },{"NORMAL",{Type::Float,3} } },
				{ {Type::UnsignedNormalizedFloat,4} }, true, false, PrimitiveTopology::Triangle
			);
		}

		PipelineState& GetPipelineState() noexcept {
			return pipelineState;
		}
		RootSignature& GetRootSignature() noexcept {
			return rootSignature;
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

		Command command{};
		command.Initialize(&device);

		auto swapChain = command.CreateSwapChain(&device, hwnd);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> rtvDescriptorHeap{};
		rtvDescriptorHeap.Initialize(&device, 2);
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(0));
		rtvDescriptorHeap.PushBackView(&device, &swapChain.GetFrameBuffer(1));


		ConstantBuffer sceneDataConstantBuffer{};
		sceneDataConstantBuffer.Initialize(&device, sizeof(SceneData));

		ConstantBuffer cubemapSceneDataConstant{};
		cubemapSceneDataConstant.Initialize(&device, sizeof(CubemapSceneData));


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
		colorObjectModel.Initialize(&device, &sceneDataConstantBuffer, &cubemapSceneDataConstant);

		ColorObjectRenderer colorObjectRenderer{};
		colorObjectRenderer.Initialize(&device);


		MirrorObjectModel mirrorObjectModel{};
		mirrorObjectModel.Initialize(&device, &sceneDataConstantBuffer);

		MirrorObjectRenderer mirrorObjectRenderer{};
		mirrorObjectRenderer.Initialize(&device);


		DepthBuffer cubemapDepthBuffer{};
		cubemapDepthBuffer.Initialize(&device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, 6);

		DescriptorHeap<DescriptorHeapTypeTag::RTV> cubemapRtvDescriptorHeap{};
		cubemapRtvDescriptorHeap.Initialize(&device, 1);
		cubemapRtvDescriptorHeap.PushBackView(&device, &mirrorObjectModel.GetCubemapShaderResource());


		DepthBuffer depthBuffer{};
		depthBuffer.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 2);
		depthStencilDescriptorHeap.PushBackView(&device, &depthBuffer);
		//Ç±Ç±Ç…ViewÇçÏÇ¡ÇƒÇµÇ‹Ç®Ç§
		depthStencilDescriptorHeap.PushBackView(&device, &cubemapDepthBuffer);



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

			sceneDataConstantBuffer.Map(SceneData{ view,proj,lightDir,eye });

			for (std::size_t i = 0; i < ColorObjectNum; i++) {
				colorObjectWorlds[i] *= XMMatrixRotationY(0.02);
				colorObjectModel.MapColorBunnyData(ColorObjectData{ colorObjectWorlds[i] ,colorObjectColors[i] }, i);
			}

			auto cubemapSceneData = GetCubemapSceneData(mirrorPos);
			cubemapSceneDataConstant.Map(cubemapSceneData);

			mirrorObjectModel.MapWorld(XMMatrixScaling(3.f, 3.f, 3.f) * XMMatrixTranslation(mirrorPos.x, mirrorPos.y, mirrorPos.z));


			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);


			//Cubemap
			{
				command.SetViewport(cubemapViewport);
				command.SetScissorRect(cubemapScissorRect);

				command.Barrior(&mirrorObjectModel.GetCubemapShaderResource(), ResourceState::RenderTarget);

				command.ClearRenderTargetView(cubemapRtvDescriptorHeap.GetCPUHandle(), MirrorObjectModel::CUBEMAP_CLEAR_VALUE);
				command.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(1), 1.f);

				command.SetRenderTarget(cubemapRtvDescriptorHeap.GetCPUHandle(), depthStencilDescriptorHeap.GetCPUHandle(1));

				command.SetPipelineState(&colorObjectRenderer.GetCubemapPipelineState());
				command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
				command.SetGraphicsRootSignature(&colorObjectRenderer.GetRootSignature());
				bunnyMesh.SetMesh(&command);

				for (std::size_t i = 0; i < ColorObjectNum; i++) {
					colorObjectModel.SetDescriptorHeap(&command, i);
					bunnyMesh.Draw(&command);
				}

				command.Barrior(&mirrorObjectModel.GetCubemapShaderResource(), ResourceState::PixcelShaderResource);
			}

			//BackBuffer
			{
				command.SetViewport(viewport);
				command.SetScissorRect(scissorRect);

				command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);
				command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

				command.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);

				command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), depthStencilDescriptorHeap.GetCPUHandle());


				//ColorBunny
				{
					command.SetPipelineState(&colorObjectRenderer.GetStanderdPipelineState());
					command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
					command.SetGraphicsRootSignature(&colorObjectRenderer.GetRootSignature());
					bunnyMesh.SetMesh(&command);

					for (std::size_t i = 0; i < ColorObjectNum; i++) {
						colorObjectModel.SetDescriptorHeap(&command, i);
						bunnyMesh.Draw(&command);
					}
				}

				//MirrorSphere
				{
					command.SetPipelineState(&mirrorObjectRenderer.GetPipelineState());
					command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
					command.SetGraphicsRootSignature(&mirrorObjectRenderer.GetRootSignature());
					mirrorObjectModel.SetDescriptorHeap(&command);
					sphereMesh.SetMesh(&command);
					sphereMesh.Draw(&command);
				}

				command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::Common);
			}


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