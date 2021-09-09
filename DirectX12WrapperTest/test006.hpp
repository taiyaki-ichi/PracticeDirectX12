#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"shader.hpp"
#include"root_signature/root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/shader_resource.hpp"
#include"descriptor_heap.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"
#include"resource/map.hpp"
#include"resource/texture_upload_buffer_resource.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

namespace test006
{

	int main()
	{
		using namespace DX12;

		using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

		using VertexLayout = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;

		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

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


		shader cs{};
		cs.initialize(L"Shader/ComputeShader001.hlsl", "main", "cs_5_0");

		root_signature computeRootsignature{};
		computeRootsignature.initialize(device,
			{ {descriptor_range_type::SRV,descriptor_range_type::UAV} },
			{}
		);

		compute_pipeline_state computePipelineState{};
		computePipelineState.initialize(device, computeRootsignature, cs);

		int x, y, n;
		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> textureResource{};
		{
			std::uint8_t* data = stbi_load("../../Assets/icon.png", &x, &y, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, x, y);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < y; i++)
				for (std::uint32_t j = 0; j < x; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * y) * 4 + k];

			textureResource.initialize(device, x, y, 1, 1);

			command.reset(0);
			command.barrior(textureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, textureResource);
			command.barrior(textureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

			stbi_image_free(data);
		}

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>, resource_flag::AllowUnorderdAccess> float4ShaderResource{};
		float4ShaderResource.initialize(device, x, y, 1, 0);

		descriptor_heap_CBV_SRV_UAV computeDescriptorHeap{};
		computeDescriptorHeap.initialize(device, 2);
		computeDescriptorHeap.push_back_texture2D_SRV(device, textureResource, 1, 0, 0, 0.f);
		computeDescriptorHeap.push_back_texture2D_UAV(device, float4ShaderResource, 0, 0);


		//
		command.reset(0);
		command.barrior(float4ShaderResource, resource_state::UnorderedAccessResource);
		command.set_compute_root_signature(computeRootsignature);
		command.set_descriptor_heap(computeDescriptorHeap);
		command.set_compute_root_descriptor_table(0, computeDescriptorHeap.get_GPU_handle());
		command.set_pipeline_state(computePipelineState);
		command.barrior(float4ShaderResource, resource_state::PixcelShaderResource);
		command.dispatch(x, y, 1);
		command.close();
		command.execute();
		command.fence(0);
		command.wait(0);
	

		//
		std::array<std::array<float, 5>, 4> vertex{ {
			{-0.8f,-0.8f,0.f,0.f,1.f},
			{-0.8f,0.8f,0.f,0.f,0.f},
			{0.8f,-0.8f,0.f,1.f,1.f},
			{0.8f,0.8f,0.f,1.f,0.f}
			} };

		std::array<std::uint32_t, 6> index{
			0,1,2,2,1,3
		};

		vertex_buffer_resource<VertexLayout> vertexBuffer{};
		vertexBuffer.initialize(device, vertex.size());

		auto vertexBufferMappedResource = map(vertexBuffer);
		for (std::uint32_t i = 0; i < vertex.size(); i++) {
			for (std::uint32_t j = 0; j < 3; j++)
				vertexBufferMappedResource.reference<0>(i, j) = vertex[i][j];
			for (std::uint32_t j = 0; j < 2; j++)
				vertexBufferMappedResource.reference<1>(i, j) = vertex[i][j + 3];
		}

		index_buffer_resource<format<component_type::UINT,32,1>> indexBuffer{};
		indexBuffer.initialize(device, index.size());

		auto indexBufferMappedResource = map(indexBuffer);
		for (std::uint32_t i = 0; i < index.size(); i++)
			indexBufferMappedResource.reference(i) = index[i];

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 1);
		descriptorHeap.push_back_texture2D_SRV(device, float4ShaderResource, 1, 0, 0, 0.f);

		root_signature rootSignature{};
		rootSignature.initialize(device, { {descriptor_range_type::SRV} }, { StaticSamplerType::Standard });

		shader vertexShader{};
		vertexShader.initialize(L"Shader/VertexShader002.hlsl", "main", "vs_5_0");

		shader pixelShader{};
		pixelShader.initialize(L"Shader/PixelShader002.hlsl", "main", "ps_5_0");

		graphics_pipeline_state<VertexLayout,format_tuple<FrameBufferFormat>> pipelineState{};
		pipelineState.initialize(device, rootSignature, { &vertexShader, &pixelShader },
			{ "POSITION","TEXCOOD" }, false, false, primitive_topology::TRIANGLE
		);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		while (update_window()) {

			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);

			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

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
		}
		command.wait_all(device);

		return 0;
	}


}