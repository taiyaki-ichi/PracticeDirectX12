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
#include"Resource/shader_resource.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"Resource/map.hpp"

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

	inline std::pair<std::vector<Vertex>,std::vector<std::uint32_t>> GetGroundPatch()
	{
		std::vector<Vertex> vertexList{};
		std::vector<std::uint32_t> indexList{};

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

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(&device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &swapChain.GetFrameBuffer(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &swapChain.GetFrameBuffer(1), 0, 0);

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
			{ {"POSITION",component_type::FLOAT,32,3},{"TEXCOOD",component_type::FLOAT,32,2} },
			{ {component_type::UNSIGNED_NORMALIZE_FLOAT,8,4} }, true, false, PrimitiveTopology::Patch
		);

		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.f;
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

		shader_resource<typeless_format<32, 1>,resource_flag::AllowDepthStencil> depthStencilBufferResource{};
		depthStencilBufferResource.initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, &depthClearValue);

		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(&device, 1);
		depthStencilDescriptorHeap.push_back_textre2D_DSV<component_type::FLOAT>(&device, &depthStencilBufferResource, 0);


		constant_buffer_resource sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(&device, sizeof(SceneData));

		shader_resource<typeless_format<8, 4>> heightMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/heightmap.png", &textureWidth, &textureHeight, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(&device, TextureDataPitchAlignment(textureWidth * 4) * textureHeight);
			map(&uploadResource, data, textureWidth * 4, textureHeight, TextureDataPitchAlignment(textureWidth * 4));

			heightMapTextureResource.initialize(&device, textureWidth, textureHeight, 1, 1);

			command.Reset(0);
			command.Barrior(&heightMapTextureResource, resource_state::CopyDest);
			command.CopyTexture(&device, &uploadResource, &heightMapTextureResource);
			command.Barrior(&heightMapTextureResource, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		shader_resource<typeless_format<8, 4>> normalMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/normalmap.png", &textureWidth, &textureHeight, &n, 4);
			buffer_resource uploadResource{};
			uploadResource.initialize(&device, TextureDataPitchAlignment(textureWidth * 4) * textureHeight);
			map(&uploadResource, data, textureWidth * 4, textureHeight, TextureDataPitchAlignment(textureWidth * 4));

			normalMapTextureResource.initialize(&device, textureWidth, textureHeight, 1, 1);

			command.Reset(0);
			command.Barrior(&normalMapTextureResource, resource_state::CopyDest);
			command.CopyTexture(&device, &uploadResource, &normalMapTextureResource);
			command.Barrior(&normalMapTextureResource, resource_state::PixcelShaderResource);
			command.Close();
			command.Execute();
			command.Fence(0);
			command.Wait(0);

			stbi_image_free(data);
		}

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(&device, 3);
		descriptorHeap.push_back_CBV(&device, &sceneDataConstantBuffer, sizeof(SceneData));
		descriptorHeap.push_back_texture2D_SRV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &heightMapTextureResource, 1, 0, 0, 0.f);
		descriptorHeap.push_back_texture2D_SRV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &normalMapTextureResource, 1, 0, 0, 0.f);


		auto [vertexList, indexList] = GetGroundPatch();

		vertex_buffer_resource vertexBuffer{};
		vertexBuffer.initialize(&device, vertexList.size()*sizeof(Vertex), sizeof(Vertex));
		map(&vertexBuffer, vertexList.begin(), vertexList.end());

		index_buffer_resource indexBuffer{};
		indexBuffer.initialize(&device, indexList.size()*sizeof(std::uint32_t), { component_type::UINT,32,1 });
		map(&indexBuffer, indexList.begin(),indexList.end());


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

		map(&sceneDataConstantBuffer, SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) });

		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			eye.x = len * std::cos(cnt / 60.0) - len / 3.f * 2.f;
			//eye.z = len * std::sin(cnt / 600.0);
			XMFLOAT3 t{ eye.x + 10.f,-1.f,0 };
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&t), XMLoadFloat3(&up));
			map(&sceneDataConstantBuffer, SceneData{ view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) });
			cnt++;


			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			//
			command.Barrior(&depthStencilBufferResource, resource_state::DepthWrite);

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::RenderTarget);

			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });
			command.ClearDepthView(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);
			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), depthStencilDescriptorHeap.get_CPU_handle());
			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);
			command.SetPipelineState(&pipelineState);
			command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			command.SetGraphicsRootSignature(&rootSignature);
			command.SetDescriptorHeap(&descriptorHeap);
			command.SetGraphicsRootDescriptorTable(0, descriptorHeap.get_GPU_handle());
			command.SetVertexBuffer(&vertexBuffer);
			command.SetIndexBuffer(&indexBuffer);
			command.SetPrimitiveTopology(PrimitiveTopology::Contorol4PointPatchList);
			command.DrawIndexedInstanced(indexList.size());

			command.Barrior(&swapChain.GetFrameBuffer(backBufferIndex), resource_state::Common);

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