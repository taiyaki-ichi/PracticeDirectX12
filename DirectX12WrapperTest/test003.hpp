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

//ジオメトリシェーダを使用し法線などを描画
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

		using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

		using VertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>>;

		auto hwnd = create_simple_window(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		device device{};
		device.initialize();

		command command{};
		command.initialize(device);

		swap_chain<FrameBufferFormat, 2> swapChain{};
		swapChain.initialize(command, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(1), 0, 0);

		auto [vertex, face] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bunny.off");
		
		VertexLayout::resource_type vertexBuffer{};
		vertexBuffer.initialize(device, vertex.size());
		map(&vertexBuffer, vertex.begin(), vertex.end());
		
		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, face.size() * 3);
		map(&indexBuffer, face.begin(), face.end());

		root_signature rootSignature{};
		rootSignature.initialize(device, { {descriptor_range_type::CBV} }, {});
		
		shader vertexShader{};
		vertexShader.initialize(L"Shader/VertexShader003.hlsl", "main", "vs_5_0");

		shader drawFacePixelShader{};
		drawFacePixelShader.initialize(L"Shader/PixelShader003_DrawFace.hlsl", "main", "ps_5_0");

		shader drawFaceGeometryShader{};
		drawFaceGeometryShader.initialize(L"Shader/GeometryShader003_DrawFace.hlsl", "main", "gs_5_0");

		shader drawNormalPixelShader{};
		drawNormalPixelShader.initialize(L"Shader/PixelShader003_DrawNormal.hlsl", "main", "ps_5_0");

		shader drawNormalGeometryShader{};
		drawNormalGeometryShader.initialize(L"Shader/GeometryShader003_DrawNormal.hlsl", "main", "gs_5_0");
		
		graphics_pipeline_state<VertexLayout, render_target_formats<FrameBufferFormat>> drawFacePipelineState{};
		drawFacePipelineState.initialize(device, rootSignature, { &vertexShader, &drawFacePixelShader,&drawFaceGeometryShader },
			{ "POSITION" }, true, false, primitive_topology::TRIANGLE
		);

		graphics_pipeline_state<VertexLayout, render_target_formats<FrameBufferFormat>> drawNormalPipelineState{};
		drawNormalPipelineState.initialize(device, rootSignature, { &vertexShader, &drawNormalPixelShader,&drawNormalGeometryShader },
			{ "POSITION" }, true, false, primitive_topology::TRIANGLE
		);

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { { 1.f } });
		
		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(device, 1);
		depthStencilDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);
		
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
		
		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);
		map(&sceneDataConstantBuffer, SceneData{ view,proj });
	
		
		//SceneDataをシェーダに渡す用
		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 1);
		descriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);

		
		std::size_t cnt = 0;
		while (UpdateWindow()) {

			eye.x = len * std::cos(cnt / 60.0);
			eye.z = len * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
			map(&sceneDataConstantBuffer, SceneData{ view,proj });
			cnt++;

			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);

			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });

			command.barrior(depthBuffer, resource_state::DepthWrite);
			command.clear_depth_view(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);

			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} }, depthStencilDescriptorHeap.get_CPU_handle());

			//面の描写
			{
				command.set_pipeline_state(drawFacePipelineState);
				command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
				command.set_graphics_root_signature(rootSignature);

				command.set_vertex_buffer(vertexBuffer);
				command.set_index_buffer(indexBuffer);
				command.set_descriptor_heap(descriptorHeap);
				command.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle());

				command.draw_indexed_instanced(face.size() * 3);
			}

			//法線の描写
			{
				command.set_pipeline_state(drawNormalPipelineState);
				command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
				command.set_graphics_root_signature(rootSignature);

				command.set_vertex_buffer(vertexBuffer);
				command.set_index_buffer(indexBuffer);
				command.set_descriptor_heap(descriptorHeap);
				command.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle());

				command.draw_indexed_instanced(face.size() * 3);
			}

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);

			command.close();
			command.execute();

			swapChain.present();
			command.fence(backBufferIndex);

			command.wait(swapChain.get_vcurrent_back_buffer_index());
		};

		command.wait_all(device);
		

		return 0;
	}
}