#pragma once
#include"Window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"descriptor_heap.hpp"
#include"resource/shader_resource.hpp"
#include"resource/texture_upload_buffer_resource.hpp"
#include"resource/constant_buffer_resource.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"

#include<DirectXMath.h>
#include<cmath>

namespace test010
{
	using namespace DX12;
	using namespace DirectX;

	struct ComputeData
	{
		std::uint32_t srcMipLevel;
		std::uint32_t numMipLevels;
		std::uint32_t srcDimention;
		bool isSRGB;
		std::array<float, 2> texelSize;
	};

	struct GroundData 
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		XMMATRIX world;
	};

	std::uint32_t get_src_dimention(int x, int y)
	{
		if (x % 2 == 0 && y % 2 == 0)
			return 0;
		else if (y % 2 == 0)
			return 1;
		else if (x % 2 == 0)
			return 2;
		else
			return 3;
	}

	inline int main()
	{
		constexpr std::size_t WINDOW_WIDTH = 1024;
		constexpr std::size_t WINDOW_HEIGHT = 768;

		using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;
		constexpr std::uint32_t FRAME_BUFFER_NUM = 2;

		auto hwnd = create_simple_window(L"test", WINDOW_WIDTH, WINDOW_HEIGHT);

		device device{};
		device.initialize();

		command<2> command{};
		command.initialize(device);

		swap_chain<FrameBufferFormat, FRAME_BUFFER_NUM> swapChain{};
		swapChain.initialize(command, hwnd);

		descriptor_heap_RTV rtvDescriptorHeap{};
		rtvDescriptorHeap.initialize(device, FRAME_BUFFER_NUM);
		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			rtvDescriptorHeap.push_back_texture2D_RTV(device, swapChain.get_frame_buffer(i), 0, 0);



		constexpr std::uint32_t TEXTURE_WIDTH = 1024; 
		constexpr std::uint32_t TEXTURE_HEIGHT = 1024;

		constexpr std::uint32_t MIP_LEVEL = 9;

		constexpr std::uint32_t RECT_EDGE = 32;

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>,resource_flag::ALLOW_UNORDERED_ACCESS> textureResource{};
		{
			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, TEXTURE_WIDTH, TEXTURE_HEIGHT);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < TEXTURE_HEIGHT; i++)
				for (std::uint32_t j = 0; j < TEXTURE_WIDTH; j++)
					for (std::uint32_t k = 0; k < 3; k++)
						mappedUploadResource.reference(j, i, k) = (i / RECT_EDGE % 2 == j / RECT_EDGE % 2) ? 255 : 0;

			for (std::uint32_t i = 0; i < TEXTURE_HEIGHT; i++)
				for (std::uint32_t j = 0; j < TEXTURE_WIDTH; j++)
					mappedUploadResource.reference(j, i, 3) = 255;

			textureResource.initialize(device, TEXTURE_WIDTH, TEXTURE_HEIGHT, 1, MIP_LEVEL);

			command.reset(0);
			command.barrior(textureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, textureResource);
			command.barrior(textureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);
		}

		
		constant_buffer_resource<ComputeData> computeDataConstantBuffer{};
		computeDataConstantBuffer.initialize(device);
	

		descriptor_heap_CBV_SRV_UAV computeDescriptorHeap{};
		computeDescriptorHeap.initialize(device, 2 + MIP_LEVEL);

		shader cs{};
		cs.initialize(L"Shader/ComputeShader005.hlsl", "main", "cs_5_1");

		root_signature computeRootSignature{};
		computeRootSignature.initialize(device, { {descriptor_range_type::SRV,descriptor_range_type::CBV,
				descriptor_range_type::UAV,descriptor_range_type::UAV,descriptor_range_type::UAV,descriptor_range_type::UAV} },
			{ static_sampler_desc::clamp_liner() });

		compute_pipeline_state computePipelineState{};
		computePipelineState.initialize(device, computeRootSignature, cs);

		//
		{
			computeDescriptorHeap.push_back_texture2D_SRV(device, textureResource, 1, 0, 0, 0.f);
			computeDescriptorHeap.push_back_CBV(device, computeDataConstantBuffer);
			for (std::uint32_t i = 0; i < 4; i++)
				computeDescriptorHeap.push_back_texture2D_UAV(device, textureResource, i + 1, 0);

			auto tmp = map(computeDataConstantBuffer);

			tmp.reference().numMipLevels = 4;
			tmp.reference().srcMipLevel = 0;
			tmp.reference().srcDimention = get_src_dimention(TEXTURE_WIDTH, TEXTURE_HEIGHT);
			tmp.reference().isSRGB = true;
			tmp.reference().texelSize = { 1.f / static_cast<float>(TEXTURE_WIDTH) * 2.f,1.f / static_cast<float>(TEXTURE_HEIGHT) * 2.f };

			command.reset(0);

			command.set_compute_root_signature(computeRootSignature);
			command.set_descriptor_heap(computeDescriptorHeap);
			command.set_compute_root_descriptor_table(0, computeDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(computePipelineState);
			command.dispatch(TEXTURE_WIDTH / 8 + 1, TEXTURE_HEIGHT / 8 + 1, 1);

			command.close();
			command.execute();

			command.fence(0);
			command.wait(0);
		}
		
		computeDescriptorHeap.reset();
		
		{
			computeDescriptorHeap.push_back_texture2D_SRV(device, textureResource, 1, 0, 0, 0.f);
			computeDescriptorHeap.push_back_CBV(device, computeDataConstantBuffer);
			for (std::uint32_t i = 0; i < 4; i++)
				computeDescriptorHeap.push_back_texture2D_UAV(device, textureResource, i + 5, 0);

			auto tmp = map(computeDataConstantBuffer);

			tmp.reference().numMipLevels = 4;
			tmp.reference().srcMipLevel = 4;
			tmp.reference().srcDimention = get_src_dimention(TEXTURE_WIDTH / std::powf(2.f, 4.f), TEXTURE_HEIGHT / std::powf(2.f, 4.f));
			tmp.reference().isSRGB = true;
			tmp.reference().texelSize = { 1.f / static_cast<float>(TEXTURE_WIDTH) * std::powf(2.f,4.f),1.f / static_cast<float>(TEXTURE_HEIGHT) * std::powf(2.f,4.f) };

			command.reset(0);

			command.set_compute_root_signature(computeRootSignature);
			command.set_descriptor_heap(computeDescriptorHeap);
			command.set_compute_root_descriptor_table(0, computeDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(computePipelineState);
			command.dispatch(TEXTURE_WIDTH / std::powf(2.f, 3.f) / 8 + 1, TEXTURE_HEIGHT / std::powf(2.f, 3.f) / 8 + 1, 1);

			command.close();
			command.execute();

			command.fence(0);
			command.wait(0);
		}
		//


		using VertexFormat = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;

		vertex_buffer_resource<VertexFormat> vertexBuffer{};
		vertexBuffer.initialize(device, 4);

		auto vertexMappedResource = map(vertexBuffer);

		vertexMappedResource.reference<0>(0, 0) = -1.f;
		vertexMappedResource.reference<0>(0, 1) = 0.f;
		vertexMappedResource.reference<0>(0, 2) = -1.f;
		vertexMappedResource.reference<1>(0, 0) = 0.f;
		vertexMappedResource.reference<1>(0, 1) = 1.f;

		vertexMappedResource.reference<0>(1, 0) = -1.f;
		vertexMappedResource.reference<0>(1, 1) = 0.f;
		vertexMappedResource.reference<0>(1, 2) = 1.f;
		vertexMappedResource.reference<1>(1, 0) = 0.f;
		vertexMappedResource.reference<1>(1, 1) = 0.f;

		vertexMappedResource.reference<0>(2, 0) = 1.f;
		vertexMappedResource.reference<0>(2, 1) = 0.f;
		vertexMappedResource.reference<0>(2, 2) = -1.f;
		vertexMappedResource.reference<1>(2, 0) = 1.f;
		vertexMappedResource.reference<1>(2, 1) = 1.f;

		vertexMappedResource.reference<0>(3, 0) = 1.f;
		vertexMappedResource.reference<0>(3, 1) = 0.f;
		vertexMappedResource.reference<0>(3, 2) = 1.f;
		vertexMappedResource.reference<1>(3, 0) = 1.f;
		vertexMappedResource.reference<1>(3, 1) = 0.f;


		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, 6);

		auto indexMappedResource = map(indexBuffer);

		indexMappedResource.reference(0) = 0;
		indexMappedResource.reference(1) = 1;
		indexMappedResource.reference(2) = 2;
		indexMappedResource.reference(3) = 2;
		indexMappedResource.reference(4) = 1;
		indexMappedResource.reference(5) = 3;


		constant_buffer_resource<GroundData> groundDataConstantBuffer{};
		groundDataConstantBuffer.initialize(device);

		shader vertexShader{};
		vertexShader.initialize(L"Shader/Ground5/VertexShader.hlsl", "main", "vs_5_1");

		shader pixelShader{};
		pixelShader.initialize(L"Shader/Ground5/PixelShader.hlsl", "main", "ps_5_1");

		root_signature rootSignature{};
		rootSignature.initialize(device, { {descriptor_range_type::SRV,descriptor_range_type::CBV} },
			{ static_sampler_desc::wrap_anisotropic() });

		graphics_pipeline_state<VertexFormat,format_tuple<FrameBufferFormat>> graphicsPipelineState{};
		graphicsPipelineState.initialize(device, rootSignature, { &vertexShader,&pixelShader },
			{ "POSITION", "TEXCOOD" }, false, false, primitive_topology::TRIANGLE);

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 2);
		descriptorHeap.push_back_texture2D_SRV(device, textureResource, MIP_LEVEL, 0, 0, 0.f);
		descriptorHeap.push_back_CBV(device, groundDataConstantBuffer);


		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		float len = 1.f;
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


		auto groundDataMappedResource = map(groundDataConstantBuffer);
		groundDataMappedResource.reference().view = view;
		groundDataMappedResource.reference().proj = proj;
		groundDataMappedResource.reference().world = XMMatrixScaling(10, 10, 10);


		while (update_window())
		{
			auto backBufferIndex = swapChain.get_current_back_buffer_index();
			command.reset(backBufferIndex);

			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.,0.,0.,1.0 });
			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} });

			command.set_pipeline_state(graphicsPipelineState);
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

			command.wait(swapChain.get_current_back_buffer_index());
		}

		command.wait_all(device);

		
		return 0;
	}


}