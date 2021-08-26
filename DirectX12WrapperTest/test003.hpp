#pragma once
#include"Window.hpp"
#include"Device.hpp"
#include"Command.hpp"
#include"SwapChain.hpp"
#include"RootSignature/RootSignature.hpp"
#include"PipelineState.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"Resource/constant_buffer_resource.hpp"
#include"Resource/shader_resource.hpp"
#include"Resource/map.hpp"

#include"OffLoader.hpp"

#include<array>
#include<cmath>
#include<DirectXMath.h>

//�W�I���g���V�F�[�_���g�p���@���Ȃǂ�`��
namespace test003
{
	struct SceneData{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
	};

	inline int main()
	{
		using namespace DX12;
		using namespace DirectX;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		auto hwnd = CreateSimpleWindow(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		Device device{};
		device.Initialize();

		Command command{};
		command.Initialize(&device);

		auto swapChain = command.CreateSwapChain(&device, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(&device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &swapChain.get_frame_buffer_resource(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV<component_type::UNSIGNED_NORMALIZE_FLOAT>(&device, &swapChain.get_frame_buffer_resource(1), 0, 0);

		auto [vertex, face] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bunny.off");
		
		vertex_buffer_resource vertexBuffer{};
		vertexBuffer.initialize(&device, sizeof(float) * vertex.size() * 3, sizeof(vertex[0]));
		map(&vertexBuffer, vertex.begin(), vertex.end());
		
		index_buffer_resource indexBuffer{};
		indexBuffer.initialize(&device, sizeof(float) * face.size() * 3, { component_type::UINT,32,1 });
		map(&indexBuffer, face.begin(), face.end());

		RootSignature rootSignature{};
		rootSignature.Initialize(&device, { {DescriptorRangeType::CBV} }, {});
		
		Shader vertexShader{};
		vertexShader.Intialize(L"Shader/VertexShader003.hlsl", "main", "vs_5_0");

		Shader drawFacePixelShader{};
		drawFacePixelShader.Intialize(L"Shader/PixelShader003_DrawFace.hlsl", "main", "ps_5_0");

		Shader drawFaceGeometryShader{};
		drawFaceGeometryShader.Intialize(L"Shader/GeometryShader003_DrawFace.hlsl", "main", "gs_5_0");

		Shader drawNormalPixelShader{};
		drawNormalPixelShader.Intialize(L"Shader/PixelShader003_DrawNormal.hlsl", "main", "ps_5_0");

		Shader drawNormalGeometryShader{};
		drawNormalGeometryShader.Intialize(L"Shader/GeometryShader003_DrawNormal.hlsl", "main", "gs_5_0");
		
		PipelineState drawFacePipelineState{};
		drawFacePipelineState.Initialize(&device, &rootSignature, { &vertexShader, &drawFacePixelShader,&drawFaceGeometryShader },
			{ {"POSITION", component_type::FLOAT,32,3} }, { {component_type::UNSIGNED_NORMALIZE_FLOAT,8,4} }, true, false, PrimitiveTopology::Triangle
		);

		PipelineState drawNormalPipelineState{};
		drawNormalPipelineState.Initialize(&device, &rootSignature, { &vertexShader, &drawNormalPixelShader,&drawNormalGeometryShader },
			{ {"POSITION", component_type::FLOAT,32,3} }, { {component_type::UNSIGNED_NORMALIZE_FLOAT,8,4} }, true, false, PrimitiveTopology::Triangle
		);
		
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.f;
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthClearValue.Color[0] = 1.f;
		depthClearValue.Color[1] = 1.f;
		depthClearValue.Color[2] = 1.f;
		depthClearValue.Color[3] = 1.f;
		
		shader_resource<typeless_format<32,1>,resource_flag::AllowDepthStencil> depthBuffer{};
		depthBuffer.initialize(&device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, &depthClearValue);
		
		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(&device, 1);
		depthStencilDescriptorHeap.push_back_textre2D_DSV<component_type::FLOAT>(&device, &depthBuffer, 0);
		
		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		float len = 0.15;
		XMFLOAT3 eye{ 0,len,len };
		XMFLOAT3 target{ 0,0.1,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.01f,
			100.f
		);
		
		constent_buffer_resource sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(&device, sizeof(SceneData));
		map(&sceneDataConstantBuffer, SceneData{ view,proj });
	
		
		//SceneData���V�F�[�_�ɓn���p
		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(&device, 1);
		descriptorHeap.push_back_CBV(&device, &sceneDataConstantBuffer, sizeof(SceneData));

		
		std::size_t cnt = 0;
		while (UpdateWindow()) {

			eye.x = len * std::cos(cnt / 60.0);
			eye.z = len * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			map(&sceneDataConstantBuffer, SceneData{ view,proj });
			cnt++;

			auto backBufferIndex = swapChain.GetCurrentBackBufferIndex();
			command.Reset(backBufferIndex);

			command.Barrior(&depthBuffer, resource_state::DepthWrite);

			command.SetViewport(viewport);
			command.SetScissorRect(scissorRect);

			command.Barrior(&swapChain.get_frame_buffer_resource(backBufferIndex), resource_state::RenderTarget);
			command.ClearRenderTargetView(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });

			command.ClearDepthView(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);

			command.SetRenderTarget(rtvDescriptorHeap.get_CPU_handle(backBufferIndex),depthStencilDescriptorHeap.get_CPU_handle());

			//�ʂ̕`��
			{
				command.SetPipelineState(&drawFacePipelineState);
				command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
				command.SetGraphicsRootSignature(&rootSignature);

				command.SetVertexBuffer(&vertexBuffer);
				command.SetIndexBuffer(&indexBuffer);
				command.SetDescriptorHeap(&descriptorHeap);
				command.SetGraphicsRootDescriptorTable(0, descriptorHeap.get_GPU_handle());

				command.DrawIndexedInstanced(face.size() * 3);
			}

			//�@���̕`��
			{
				command.SetPipelineState(&drawNormalPipelineState);
				command.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
				command.SetGraphicsRootSignature(&rootSignature);

				command.SetVertexBuffer(&vertexBuffer);
				command.SetIndexBuffer(&indexBuffer);
				command.SetDescriptorHeap(&descriptorHeap);
				command.SetGraphicsRootDescriptorTable(0, descriptorHeap.get_GPU_handle());

				command.DrawIndexedInstanced(face.size() * 3);
			}

			command.Barrior(&swapChain.get_frame_buffer_resource(backBufferIndex), resource_state::Common);

			command.Close();
			command.Execute();

			swapChain.Present();
			command.Fence(backBufferIndex);

			command.Wait(swapChain.GetCurrentBackBufferIndex());
		};

		command.WaitAll(&device);
		

		return 0;
	}
}