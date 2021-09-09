#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"shader.hpp"
#include"root_signature/root_signature.hpp"
#include"pipeline_state.hpp"
#include"descriptor_heap.hpp"
#include"resource/constant_buffer_resource.hpp"
#include"resource/shader_resource.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"
#include"resource/map.hpp"
#include"resource/texture_upload_buffer_resource.hpp"

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

	using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

	using VertexFormatTuple = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 2>>;

	inline std::pair<std::vector<std::array<float,5>>,std::vector<std::uint32_t>> GetGroundPatch()
	{
		std::vector<std::array<float, 5>> vertexList{};
		std::vector<std::uint32_t> indexList{};

		constexpr float EDGE = 200.f;
		constexpr std::size_t DIVIDE = 10;
		constexpr auto ROWS = DIVIDE + 1;

		vertexList.reserve((DIVIDE + 1) * (DIVIDE + 1));
		for (std::size_t z = 0; z < DIVIDE + 1; z++) {
			for (std::size_t x = 0; x < DIVIDE + 1; x++) {
				auto posX = EDGE * x / DIVIDE;
				auto posZ = EDGE * z / DIVIDE;
				vertexList.push_back({ posX,0.f,posZ,posX / EDGE,posZ / EDGE });
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
			v[0] -= EDGE / 2.f;
			v[2] -= EDGE / 2.f;
		}

		return { std::move(vertexList),std::move(indexList) };
	}


	inline int main()
	{
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

		shader vs{};
		vs.initialize(L"Shader/Ground/VertexShader.hlsl", "main", "vs_5_0");

		shader ps{};
		ps.initialize(L"Shader/Ground/PixelShader.hlsl", "main", "ps_5_0");

		shader hs{};
		hs.initialize(L"Shader/Ground/HullShader.hlsl", "main", "hs_5_0");

		shader ds{};
		ds.initialize(L"Shader/Ground/DomainShader.hlsl", "main", "ds_5_0");

		root_signature rootSignature{};
		rootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::SRV,descriptor_range_type::SRV} },
			{ StaticSamplerType::Standard }
		);

		graphics_pipeline_state<VertexFormatTuple,format_tuple<FrameBufferFormat>> pipelineState{};
		pipelineState.initialize(device, rootSignature, { &vs, &ps,nullptr,&hs, &ds },
			{ "POSITION","TEXCOOD" }, true, false, primitive_topology::PATCH
		);

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> depthStencilBufferResource{};
		depthStencilBufferResource.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { {1.f} });

		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(device, 1);
		depthStencilDescriptorHeap.push_back_texture2D_DSV(device, depthStencilBufferResource, 0);


		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> heightMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/heightmap.png", &textureWidth, &textureHeight, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, textureWidth, textureHeight);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < textureHeight; i++)
				for (std::uint32_t j = 0; j < textureWidth; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * textureWidth) * 4 + k];


			heightMapTextureResource.initialize(device, textureWidth, textureHeight, 1, 1);

			command.reset(0);
			command.barrior(heightMapTextureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, heightMapTextureResource);
			command.barrior(heightMapTextureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

			stbi_image_free(data);
		}

		shader_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> normalMapTextureResource{};
		{
			int textureWidth, textureHeight, n;
			std::uint8_t* data = stbi_load("../../Assets/normalmap.png", &textureWidth, &textureHeight, &n, 4);

			texture_upload_buffer_resource<format<component_type::UINT, 8, 4>> uploadResource{};
			uploadResource.initialize(device, textureWidth, textureHeight);

			auto mappedUploadResource = map(uploadResource);
			for (std::uint32_t i = 0; i < textureHeight; i++)
				for (std::uint32_t j = 0; j < textureWidth; j++)
					for (std::uint32_t k = 0; k < 4; k++)
						mappedUploadResource.reference(j, i, k) = data[(j + i * textureWidth) * 4 + k];

			normalMapTextureResource.initialize(device, textureWidth, textureHeight, 1, 1);

			command.reset(0);
			command.barrior(normalMapTextureResource, resource_state::CopyDest);
			command.copy_texture(device, uploadResource, normalMapTextureResource);
			command.barrior(normalMapTextureResource, resource_state::PixcelShaderResource);
			command.close();
			command.execute();
			command.fence(0);
			command.wait(0);

			stbi_image_free(data);
		}

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};
		descriptorHeap.initialize(device, 3);
		descriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		descriptorHeap.push_back_texture2D_SRV(device, heightMapTextureResource, 1, 0, 0, 0.f);
		descriptorHeap.push_back_texture2D_SRV(device, normalMapTextureResource, 1, 0, 0, 0.f);


		auto [vertexList, indexList] = GetGroundPatch();

		vertex_buffer_resource<VertexFormatTuple> vertexBuffer{};
		vertexBuffer.initialize(device, vertexList.size());

		auto vertexBufferMappedResource = map(vertexBuffer);
		for (std::uint32_t i = 0; i < vertexList.size(); i++) {
			for (std::uint32_t j = 0; j < 3; j++)
				vertexBufferMappedResource.reference<0>(i, j) = vertexList[i][j];
			for (std::uint32_t j = 0; j < 2; j++)
				vertexBufferMappedResource.reference<1>(i, j) = vertexList[i][j + 3];
		}

		index_buffer_resource<format<component_type::UINT, 32, 1>> indexBuffer{};
		indexBuffer.initialize(device, indexList.size());

		auto indexBufferMappedResource = map(indexBuffer);
		for (std::uint32_t i = 0; i < indexList.size(); i++)
			indexBufferMappedResource.reference(i) = indexList[i];


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

		auto sceneDataMappedResource = map(sceneDataConstantBuffer);
		sceneDataMappedResource.reference() = { view,proj,XMMatrixIdentity(),eye,XMFLOAT4(16.f,100.f,4.f,0.f) };


		std::size_t cnt = 0;
		while (update_window())
		{
			eye.x = len * std::cos(cnt / 60.0) - len / 3.f * 2.f;
			//eye.z = len * std::sin(cnt / 600.0);
			XMFLOAT3 t{ eye.x + 10.f,-1.f,0 };
			auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&t), XMLoadFloat3(&up));

			sceneDataMappedResource.reference().eye = eye;
			sceneDataMappedResource.reference().view = view;

			cnt++;


			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();
			command.reset(backBufferIndex);


			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

			command.barrior(depthStencilBufferResource, resource_state::DepthWrite);
			command.clear_depth_view(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);

			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} }, depthStencilDescriptorHeap.get_CPU_handle());
			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);
			command.set_pipeline_state(pipelineState);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.set_graphics_root_signature(rootSignature);
			command.set_descriptor_heap(descriptorHeap);
			command.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle());
			command.set_vertex_buffer(vertexBuffer);
			command.set_index_buffer(indexBuffer);
			command.set_primitive_topology(primitive_topology::CONTOROL_4_POINT_PATCH_LIST);
			command.draw_indexed_instanced(indexList.size());

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