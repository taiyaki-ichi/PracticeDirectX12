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
#include"Resource/ShaderResource.hpp"
#include"Resource/VertexBufferResource.hpp"
#include"Resource/IndexBufferResource.hpp"

#include<vector>
#include<cmath>

#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include<stb_image.h>

#include<DirectXMath.h>

namespace test005
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

	struct Vertex {
		float x, y, z;
		float uvX, uvY;
	};

	inline std::pair<std::vector<Vertex>,std::vector<std::uint16_t>> GetGroundPatch()
	{
		std::vector<Vertex> vertexList{};
		std::vector<std::uint16_t> indexList{};

		constexpr float EDGE = 200.f;
		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = EDGE * x / DIVIDE;
				auto posZ = EDGE * z / DIVIDE;
				vertexList.push_back(Vertex{ posX,0.f,posZ,posX / EDGE,posZ / EDGE });
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
			v.x -= EDGE / 2.f;
			v.z -= EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
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

		Shader vs{};
		vs.Intialize(L"Shader/Ground/VertexShader.hlsl", "main", "vs_5_0");

		Shader ps{};
		ps.Intialize(L"Shader/Ground/PixelShader.hlsl", "main", "ps_5_0");

		Shader hs{};
		hs.Intialize(L"Shader/Ground/HullShader.hlsl", "main", "hs_5_0");

		Shader ds{};
		ds.Intialize(L"Shader/Ground/DomainShader.hlsl", "main", "ds_5_0");

		RootSignature rootSignature{};
		rootSignature.Initialize(&device,
			{ {DescriptorRangeType::CBV,DescriptorRangeType::SRV,DescriptorRangeType::SRV} },
			{ StaticSamplerType::Standard }
		);

		PipelineState pipelineState{};
		pipelineState.Initialize(&device, &rootSignature, { &vs, &ps,nullptr,&hs, &ds },
			{ {"POSITION",{Type::Float,3}},{"TEXCOOD",{Type::Float,2}} },
			{ {Type::UnsignedNormalizedInt8,4} }, true, false, PrimitiveTopology::Patch
		);

		DepthBufferResource depthStencilBufferResource{};
		depthStencilBufferResource.Initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT);

		DescriptorHeap<DescriptorHeapTypeTag::DSV> depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.Initialize(&device, 1);
		depthStencilDescriptorHeap.PushBackView(&device, &depthStencilBufferResource);


		ConstantBufferResource sceneDataConstantBufferResource{};
		sceneDataConstantBufferResource.Initialize(&device, sizeof(SceneData));

		Float4ShaderResource heightMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/heightmap.png", &textureWidth, &textureHeight, &n, 4);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(textureWidth * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * textureHeight);
			uploadResource.Map(data, textureWidth * 4, textureHeight);
			heightMapTextureResource.Initialize(&device, textureWidth, textureHeight);

			command.Reset(0);
			command.Barrior(&heightMapTextureResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &heightMapTextureResource);
			command.Barrior(&heightMapTextureResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		Float4ShaderResource normalMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/normalmap.png", &textureWidth, &textureHeight, &n, 4);
			UploadResource uploadResource{};
			uploadResource.Initialize(&device, AlignmentSize(textureWidth * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * textureHeight);
			uploadResource.Map(data, textureWidth * 4, textureHeight);
			normalMapTextureResource.Initialize(&device, textureWidth, textureHeight);

			command.Reset(0);
			command.Barrior(&normalMapTextureResource, ResourceState::CopyDest);
			command.CopyTexture(&device, &uploadResource, &normalMapTextureResource);
			command.Barrior(&normalMapTextureResource, ResourceState::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
		descriptorHeap.Initialize(&device, 3);
		descriptorHeap.PushBackView(&device, &sceneDataConstantBufferResource);
		descriptorHeap.PushBackView(&device, &heightMapTextureResource);
		descriptorHeap.PushBackView(&device, &normalMapTextureResource);


		auto [vertexList, indexList] = GetGroundPatch();

		VertexBufferResource vertexBufferResource{};
		vertexBufferResource.Initialize(&device, sizeof(Vertex) * vertexList.size(), sizeof(Vertex));
		vertexBufferResource.Map(vertexList);

		IndexBufferResource indexBufferResource{};
		indexBufferResource.Initialize(&device, sizeof(std::uint16_t) * indexList.size());
		indexBufferResource.Map(indexList);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		float len = 50.f;
		//XMFLOAT3 eye{ len,5,len };
		XMFLOAT3 eye{ len, 2.f, 0.f };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			500.f
		);

		sceneDataConstantBufferResource.Map(SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) });

		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			eye.x = len * std::cos(cnt / 60.0) - len / 3.f * 2.f;
			//eye.z = len * std::sin(cnt / 600.0);
			XMFLOAT3 t{ eye.x + 10.f,-1.f,0 };
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&t), XMLoadFloat3(&up));
			sceneDataConstantBufferResource.Map(SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) });
			cnt++;


			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), ResourceState::RenderTarget);

			command.ClearRenderTargetView(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });
			command.ClearDepthView(depthStencilDescriptorHeap.GetCPUHandle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.GetCPUHandle(backBufferIndex), depthStencilDescriptorHeap.GetCPUHandle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);
			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TrinagleList);
			command.SetGraphicsRootSignature(&rootSignature);
			command.SetDescriptorHeap(&descriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, descriptorHeap.GetGPUHandle());
			command.SetVertexBuffer(&vertexBufferResource);
			command.SetIndexBuffer(&indexBufferResource);
			command.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
			command.DrawIndexedInstanced(indexList.size());

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