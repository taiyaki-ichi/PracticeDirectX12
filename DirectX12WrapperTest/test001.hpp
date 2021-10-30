#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/buffer_resource.hpp"
#include"resource/map_stream.hpp"

#include<array>
#include<sstream>

#include<iostream>

//ƒ|ƒŠƒSƒ“‚Ì•`ŽÊ‚Ì•`ŽÊ‚ð‚·‚é
namespace test001
{
	inline int main()
	{
		using namespace DX12;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;
		constexpr std::uint32_t FRAME_BUFFER_NUM = 2;

		auto hwnd = create_simple_window(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		device device{};
		device.initialize();

		command<1> command{};
		command.initialize(device);

		swap_chain<FrameBufferFormat, FRAME_BUFFER_NUM> swapChain{};
		swapChain.initialize(command, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, FRAME_BUFFER_NUM);
		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(i), 0, 0);

		using VertexFormatTuple = format_tuple<format<component_type::FLOAT, 32, 3>>;

		buffer_resource<VertexFormatTuple,resource_heap_property::UPLOAD> vertexBuffer{};
		vertexBuffer.initialize(device, 3);

		auto s = map(vertexBuffer);
		s << -0.8f << -0.8f << -0.f;
		s << -0.8f << 0.8f << 0.f;
		s << 0.8f << -0.8f << 0.f;

		vertexBuffer.get()->Unmap(0, nullptr);


		root_signature rootSignature{};
		rootSignature.initialize(device, {}, {});

		shader vertexShader{};
		vertexShader.initialize(L"Shader/VertexShader001.hlsl", "main", "vs_5_0");

		shader pixelShader{};
		pixelShader.initialize(L"Shader/PixelShader001.hlsl", "main", "ps_5_0");

		graphics_pipeline_state<VertexFormatTuple, format_tuple<FrameBufferFormat>> pipelineState{};
		pipelineState.initialize(device, rootSignature, { &vertexShader, &pixelShader },
			{ "POSITION" }, false, false, primitive_topology::TRIANGLE
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		std::size_t cnt = 0;
		while (update_window()) {

			auto backBufferIndex = swapChain.get_current_back_buffer_index();
			command.reset(0);

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
			command.fence(0);

			command.wait(0);
		};

		return 0;
	}
}