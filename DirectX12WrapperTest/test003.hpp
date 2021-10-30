#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/buffer_resource.hpp"
#include"resource/allow_depth_stencil_texture_resource.hpp"

#include"resource/map_stream.hpp"

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

		using VertexFormatTuple = format_tuple<format<component_type::FLOAT, 32, 3>>;

		auto hwnd = create_simple_window(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		device device{};
		device.initialize();

		command<2> command{};
		command.initialize(device);

		swap_chain<FrameBufferFormat, 2> swapChain{};
		swapChain.initialize(command, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, 2);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(0), 0, 0);
		rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(1), 0, 0);

		auto [vertex, index] = OffLoader::LoadTriangularMeshFromOffFile2<float, std::uint32_t>("../../Assets/bunny.off");
		
		buffer_resource<format<component_type::FLOAT, 32, 3>, resource_heap_property::DEFAULT> vertexBuffer{};
		vertexBuffer.initialize(device, vertex.size() / 3);
		{
			buffer_resource<format<component_type::FLOAT, 32, 1>,resource_heap_property::UPLOAD> uploadVertexBuffer{};
			uploadVertexBuffer.initialize(device, vertex.size());
			auto stream = map(uploadVertexBuffer);
			for (auto& v : vertex)
				stream << v;

			command.reset(0);
			command.barrior(vertexBuffer, resource_state::CopyDest);
			command.copy_resource(uploadVertexBuffer, vertexBuffer);
			command.barrior(vertexBuffer, resource_state::Common);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);
		}

		buffer_resource<format<component_type::UINT, 32, 1>, resource_heap_property::DEFAULT> indexBuffer{};
		indexBuffer.initialize(device, index.size());
		{
			buffer_resource<format<component_type::UINT, 32, 1>, resource_heap_property::UPLOAD> uploadIndexBuffer{};
			uploadIndexBuffer.initialize(device, index.size());
			auto stream = map(uploadIndexBuffer);
			for (auto& i : index)
				stream << i;

			command.reset(0);
			command.barrior(indexBuffer, resource_state::CopyDest);
			command.copy_resource(uploadIndexBuffer, indexBuffer);
			command.barrior(indexBuffer, resource_state::Common);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);
		}

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
		
		graphics_pipeline_state<VertexFormatTuple, format_tuple<FrameBufferFormat>> drawFacePipelineState{};
		drawFacePipelineState.initialize(device, rootSignature, { &vertexShader, &drawFacePixelShader,&drawFaceGeometryShader },
			{ "POSITION" }, true, false, primitive_topology::TRIANGLE
		);

		graphics_pipeline_state<VertexFormatTuple, format_tuple<FrameBufferFormat>> drawNormalPipelineState{};
		drawNormalPipelineState.initialize(device, rootSignature, { &vertexShader, &drawNormalPixelShader,&drawNormalGeometryShader },
			{ "POSITION" }, true, false, primitive_topology::TRIANGLE
		);

		allow_depth_stencil_texture_2D_resource<format<component_type::FLOAT, 32, 1>> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, 1.f, 0);
		
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
		
		buffer_resource<SceneData,resource_heap_property::UPLOAD> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device, 1);

		SceneData sceneData{ view,proj };

		//SceneDataをシェーダに渡す用
		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 1);
		descriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);

		
		std::size_t cnt = 0;
		while (update_window()) {

			eye.x = len * std::cos(cnt / 60.0);
			eye.z = len * std::sin(cnt / 60.0);
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

			sceneData.view = view;
			auto sceneDataMapStream = map(sceneDataConstantBuffer);
			sceneDataMapStream << sceneData;

			cnt++;

			auto backBufferIndex = swapChain.get_current_back_buffer_index();
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

				command.draw_indexed_instanced(index.size());
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

				command.draw_indexed_instanced(index.size());
			}

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);

			command.close();
			command.execute();

			swapChain.present();
			command.fence(backBufferIndex);

			command.wait(swapChain.get_current_back_buffer_index());
		};

		command.wait_all(device);
		

		return 0;
	}
}