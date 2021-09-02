#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"root_signature/root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/map.hpp"

#include<array>


//ƒ|ƒŠƒSƒ“‚Ì•`ŽÊ‚Ì•`ŽÊ‚ð‚·‚é
namespace test001
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

		using VertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>>;

		std::array<VertexLayout::struct_type, 3> vertex{
			{{-0.8f,-0.8f,0.f},{-0.8f,0.8f,0.f},{0.8f,-0.8f,0.f}}
		};

		vertex_buffer_resource<format<component_type::FLOAT, 32, 3>> vertexBuffer{};
		vertexBuffer.initialize(device, vertex.size());
		map(&vertexBuffer, vertex);

		root_signature rootSignature{};
		rootSignature.initialize(device, {}, {});

		shader vertexShader{};

		vertexShader.initialize(L"Shader/VertexShader001.hlsl", "main", "vs_5_0");

		shader pixelShader{};
		pixelShader.initialize(L"Shader/PixelShader001.hlsl", "main", "ps_5_0");

		graphics_pipeline_state<VertexLayout,render_target_formats<FrameBufferFormat>> pipelineState{};
		pipelineState.initialize(device, rootSignature, { &vertexShader, &pixelShader },
			{ "POSITION" }, false, false, primitive_topology::TRIANGLE
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		std::size_t cnt = 0;
		while (UpdateWindow()) {

			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);

			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.0f });

			command.set_render_target({ { rtvDescriptorHeap.get_CPU_handle(backBufferIndex) } });

			command.set_pipeline_state(pipelineState);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.set_graphics_root_signature(rootSignature);

			command.set_vertex_buffer(vertexBuffer);
			command.draw_instanced(3);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);

			command.close();
			command.execute();

			swapChain.present();
			command.fence(backBufferIndex);

			command.wait(swapChain.get_vcurrent_back_buffer_index());

			map(&vertexBuffer, vertex);
		};


		command.wait_all(device);
		return 0;
	}
}