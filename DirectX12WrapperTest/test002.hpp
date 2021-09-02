#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"
#include"resource/map.hpp"
#include"root_signature/root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/shader_resource.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

#include<array>

//テクスチャの表示をする
namespace test002
{

	inline int main()
	{
		using namespace DX12;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

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

		using VertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;

		std::array<VertexLayout::struct_type, 4> vertex{ {
			{{-0.8f,-0.8f,0.f},{0.f,1.f}},
			{{-0.8f,0.8f,0.f},{0.f,0.f}},
			{{0.8f,-0.8f,0.f},{1.f,1.f}},
			{{0.8f,0.8f,0.f},{1.f,0.f}}
			} };

		std::array<std::uint32_t, 6> index{
			0,1,2,2,1,3
		};

		VertexLayout::resource_type vertexBuffer{};
		vertexBuffer.initialize(device, vertex.size());
		map(&vertexBuffer, vertex.begin(),vertex.end());

		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, index.size());
		map(&indexBuffer, index);

		int x, y, n;
		std::uint8_t* data = stbi_load("../../Assets/icon.png", &x, &y, &n, 0);

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> textureResource{};
		{
			buffer_resource uploadResource{};
			uploadResource.initialize(device, texture_data_pitch_alignment(x * 4) * y);
			map(&uploadResource, data, x * 4, y, texture_data_pitch_alignment(x * 4));

			textureResource.initialize(device, x, y, 1, 1);

			command.reset(0);
			command.barrior(textureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, textureResource);
			command.barrior(textureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);
		}

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 1);

		descriptorHeap.push_back_texture2D_SRV(device, textureResource, 1, 0, 0, 0.f);

		root_signature rootSignature{};
		rootSignature.initialize(device, { {descriptor_range_type::SRV} }, { StaticSamplerType::Standard });

		shader vertexShader{};
		vertexShader.initialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		shader pixelShader{};
		pixelShader.initialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		graphics_pipeline_state<VertexLayout,render_target_formats<FrameBufferFormat>> pipelineState{};
		pipelineState.initialize(device, rootSignature, { &vertexShader, &pixelShader },
			{ "POSITION", "TEXCOOD" }
			, false, false, primitive_topology::TRIANGLE
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };


		while (UpdateWindow()) {
			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);

			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });

			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} });

			command.set_pipeline_state(pipelineState);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.set_graphics_root_signature(rootSignature);

			command.set_descriptor_heap(descriptorHeap);
			command.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle());
			command.set_vertex_buffer(vertexBuffer);
			command.set_index_buffer(indexBuffer);
			command.draw_indexed_instanced(6);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);

			command.close();
			command.execute();

			swapChain.present();
			command.fence(backBufferIndex);

			command.wait(swapChain.get_vcurrent_back_buffer_index());
		};

		stbi_image_free(data);
		command.wait_all(device);

		return 0;
	}
}