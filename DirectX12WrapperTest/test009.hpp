#pragma once
#include"window.hpp"
#include"device.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"root_signature/root_signature.hpp"
#include"pipeline_state.hpp"
#include"resource/constant_buffer_resource.hpp"
#include"resource/shader_resource.hpp"
#include"resource/map.hpp"

#include<array>

#include<DirectXMath.h>

#include"OffLoader.hpp"
#include"utility.hpp"

//ƒ|ƒŠƒSƒ“‚Ì•`ŽÊ‚Ì•`ŽÊ‚ð‚·‚é
namespace test009
{
	using namespace DirectX;
	using namespace DX12;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct SceneData
	{
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 eye;
		XMFLOAT4 lightDir;
		XMMATRIX lightDepthViewProj;
	};

	struct GroundData
	{
		XMMATRIX world;
	};

	using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

	using BunnyVertexLayout = vertex_layout<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3>>;

	constexpr std::uint32_t BUNNY_NUM = 3;

	struct BunnyData {
		std::array<XMMATRIX, BUNNY_NUM> world;
	};

	constexpr std::uint32_t SHADOW_MAP_EDGE = 1024;

	struct ShadowMapData
	{
		float width;
		float height;
	};

	inline int main()
	{
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


		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, { {1.f} });

		descriptor_heap_DSV dsvDescriptorHeap{};
		dsvDescriptorHeap.initialize(device, 1);
		dsvDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);


		//‹¤—L‚·‚éSceneData‚ÌConstantBuffer
		constant_buffer_resource<SceneData> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device);



		//
		//ShadowMap
		//


		//float
		shader_resource<format<component_type::FLOAT, 32, 2>, resource_flag::AllowRenderTarget> shadowMap{};
		shadowMap.initialize(device, SHADOW_MAP_EDGE, SHADOW_MAP_EDGE, 1, 1, { {1.f,1.f} });

		//float
		shader_resource<format<component_type::FLOAT, 32, 2>, resource_flag::AllowUnorderdAccess> gaussianBlurShadowMap{};
		gaussianBlurShadowMap.initialize(device, SHADOW_MAP_EDGE, SHADOW_MAP_EDGE, 1, 1);

		constant_buffer_resource<ShadowMapData> shadowMapDataConstantBuffer{};
		shadowMapDataConstantBuffer.initialize(device);
		map(&shadowMapDataConstantBuffer, ShadowMapData{ SHADOW_MAP_EDGE,SHADOW_MAP_EDGE });

		shader_resource<format<component_type::FLOAT, 32, 1>, resource_flag::AllowDepthStencil> shadowMapDepthBuffer{};
		shadowMapDepthBuffer.initialize(device, SHADOW_MAP_EDGE, SHADOW_MAP_EDGE, 1, 1, { {1.f} });

		descriptor_heap_DSV shadowMapDSVDescriptorHeap{};
		shadowMapDSVDescriptorHeap.initialize(device, 1);
		shadowMapDSVDescriptorHeap.push_back_texture2D_DSV(device, shadowMapDepthBuffer, 0);

		descriptor_heap_RTV shadowMapRTVDescriptorHeap{};
		shadowMapRTVDescriptorHeap.initialize(device, 1);
		shadowMapRTVDescriptorHeap.push_back_texture2D_RTV(device, shadowMap, 0, 0);

		descriptor_heap_CBV_SRV_UAV computeGaussianBlurDescriptorHeap{};
		computeGaussianBlurDescriptorHeap.initialize(device, 3);
		computeGaussianBlurDescriptorHeap.push_back_texture2D_SRV(device, shadowMap, 1, 0, 0, 0.f);
		computeGaussianBlurDescriptorHeap.push_back_texture2D_UAV(device, gaussianBlurShadowMap, 0, 0);
		computeGaussianBlurDescriptorHeap.push_back_CBV(device, shadowMapDataConstantBuffer);

		shader computeGaussianCS{};
		computeGaussianCS.initialize(L"Shader/ComputeShader004.hlsl", "main", "cs_5_1");

		root_signature computeGaussianBlurRootSignature{};
		computeGaussianBlurRootSignature.initialize(device, { {descriptor_range_type::SRV,descriptor_range_type::UAV,descriptor_range_type::CBV} }, {});

		compute_pipeline_state computeGaussianBlurPipelineState{};
		computeGaussianBlurPipelineState.initialize(device, computeGaussianBlurRootSignature, computeGaussianCS);


		//
		//Ground
		//

		vertex_buffer_resource<format<component_type::FLOAT,32,3>> groundVertexBuffer{};
		{
			std::array<std::array<float, 3>, 4> vertex{ {
				{-1.f,0.f,-1.f},
				{-1.f,0.f,1.f},
				{1.f,0.f,-1.f},
				{1.f,0.f,1.f}
				} };
			groundVertexBuffer.initialize(device, vertex.size());
			map(&groundVertexBuffer, std::move(vertex));
		}

		index_buffer_resource<format<component_type::UINT, 32, 1>> groundIndexBuffer{};
		std::uint32_t groundIndexNum{};
		{
			std::array<std::uint32_t, 6> index{
				0,1,2,2,1,3
			};
			groundIndexBuffer.initialize(device, index.size());
			map(&groundIndexBuffer, index);
			groundIndexNum = index.size();
		}

		constant_buffer_resource<GroundData> groundDataConstantBuffer{};
		groundDataConstantBuffer.initialize(device);

		descriptor_heap_CBV_SRV_UAV groundDescriptorHeap{};
		groundDescriptorHeap.initialize(device, 3);
		groundDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		groundDescriptorHeap.push_back_CBV(device, groundDataConstantBuffer);
		//groundDescriptorHeap.PushBackView(device, &shadowMap);
		groundDescriptorHeap.push_back_texture2D_SRV(device, gaussianBlurShadowMap, 1, 0, 0, 0.f);

		root_signature groundRootSignature{};
		groundRootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::CBV,descriptor_range_type::SRV} },
			{ StaticSamplerType::Standard }
		);

		shader groundVS{};
		groundVS.initialize(L"Shader/Ground4/VertexShader.hlsl", "main", "vs_5_1");

		shader groundPS{};
		groundPS.initialize(L"Shader/Ground4/PixelShader.hlsl", "main", "ps_5_1");

		shader groundShadowMapVS{};
		groundShadowMapVS.initialize(L"Shader/Ground4/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		shader groundShadowMapPS{};
		groundShadowMapPS.initialize(L"Shader/Ground4/ShadowMapPixelShader.hlsl", "main", "ps_5_1");

		graphics_pipeline_state<vertex_layout<format<component_type::FLOAT,32,3>>,render_target_formats<FrameBufferFormat>> groundPipelineState{};
		groundPipelineState.initialize(device, groundRootSignature, { &groundVS,&groundPS },
			{ "POSITION",},true, false, primitive_topology::TRIANGLE
		);

		graphics_pipeline_state<vertex_layout<format<component_type::FLOAT, 32, 3>>, render_target_formats<format<component_type::FLOAT, 32, 2>>> groundShadowMapPipelineState{};
		groundShadowMapPipelineState.initialize(device, groundRootSignature, { &groundShadowMapVS,&groundShadowMapPS },
			{ "POSITION" }, true, false, primitive_topology::TRIANGLE
		);



		//
		//Bunny
		//

		BunnyVertexLayout::resource_type bunnyVertexBuffer{};
		index_buffer_resource<format<component_type::UINT,32,1>> bunnyIndexBuffer{};
		std::uint32_t bunnyIndexNum{};
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/bun_zipper.off");
			//auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>("../../Assets/sphere.off");
			auto normalList = GetVertexNormal(vertexList, faceList);

			std::vector<BunnyVertexLayout::struct_type> posNormalList{};
			posNormalList.reserve(vertexList.size());
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++) {
				XMStoreFloat3(&tmpFloat3, normalList[i]);
				posNormalList.push_back({ vertexList[i],{tmpFloat3.x,tmpFloat3.y,tmpFloat3.z} });
			}

			bunnyIndexNum = faceList.size() * 3;

			bunnyVertexBuffer.initialize(device, posNormalList.size());
			map(&bunnyVertexBuffer, posNormalList.begin(), posNormalList.end());

			bunnyIndexBuffer.initialize(device, faceList.size() * 3);
			map(&bunnyIndexBuffer, faceList.begin(), faceList.end());
		}

		constant_buffer_resource<BunnyData> bunnyDataConstantBuffer{};
		bunnyDataConstantBuffer.initialize(device);

		descriptor_heap_CBV_SRV_UAV bunnyDescriptorHeap{};
		bunnyDescriptorHeap.initialize(device, 3);
		bunnyDescriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
		bunnyDescriptorHeap.push_back_CBV(device, bunnyDataConstantBuffer);
		//bunnyDescriptorHeap.PushBackView(device, &shadowMap);
		bunnyDescriptorHeap.push_back_texture2D_SRV(device, gaussianBlurShadowMap, 1, 0, 0, 0.f);

		root_signature bunnyRootSignature{};
		bunnyRootSignature.initialize(device,
			{ {descriptor_range_type::CBV,descriptor_range_type::CBV,descriptor_range_type::SRV} },
			{ StaticSamplerType::Standard }
		);

		shader bunnyVS{};
		bunnyVS.initialize(L"Shader/Bunny2/VertexShader.hlsl", "main", "vs_5_1");

		shader bunnyPS{};
		bunnyPS.initialize(L"Shader/Bunny2/PixelShader.hlsl", "main", "ps_5_1");

		shader bunnyShadowMapVS{};
		bunnyShadowMapVS.initialize(L"Shader/Bunny2/ShadowMapVertexShader.hlsl", "main", "vs_5_1");

		shader bunnyShadowMapPS{};
		bunnyShadowMapPS.initialize(L"Shader/Bunny2/ShadowMapPixelShader.hlsl", "main", "ps_5_1");

		graphics_pipeline_state<BunnyVertexLayout,render_target_formats<FrameBufferFormat>> bunnyPipelineState{};
		bunnyPipelineState.initialize(device, bunnyRootSignature, { &bunnyVS,&bunnyPS },
			{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
		);

		graphics_pipeline_state<BunnyVertexLayout,render_target_formats<format<component_type::FLOAT,32,2>>> bunnyShadowMapPipelineState{};
		bunnyShadowMapPipelineState.initialize(device, bunnyRootSignature, { &bunnyShadowMapVS ,&bunnyShadowMapPS },
			{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
		);

		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT shadowMapViewport{ 0,0, static_cast<float>(SHADOW_MAP_EDGE),static_cast<float>(SHADOW_MAP_EDGE),0.f,1.f };
		D3D12_RECT shadowMapScissorRect{ 0,0,static_cast<LONG>(SHADOW_MAP_EDGE),static_cast<LONG>(SHADOW_MAP_EDGE) };

		XMFLOAT3 eye{ 50, 20.f, 0.f };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			500.f
		);
		XMFLOAT3 lightDir{ 0.1f,1.f,1.f };

		auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&lightDir))
			* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

		XMMATRIX shadowMapViewProj = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(100, 100, -100.f, 100.f);

		map(&sceneDataConstantBuffer, SceneData{ view,proj,{eye.x,eye.y,eye.z,0.f}, {lightDir.x,lightDir.y,lightDir.z,0.f},shadowMapViewProj });

		map(&groundDataConstantBuffer, GroundData{ XMMatrixScaling(100.f,100.f,100.f) });

		BunnyData bunnyData{};
		//for (std::size_t i = 0; i < BUNNY_NUM; i++)
			//bunnyData.world[i] = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixTranslation(30.f - 30.f * i, 5.f, 20.f - i * 10.f);
		//bunnyDataConstantBuffer.Map(bunnyData);


		std::size_t cnt = 0;
		while (UpdateWindow())
		{
			//
			//update
			//
			for (std::size_t i = 0; i < BUNNY_NUM; i++)
				bunnyData.world[i] = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixRotationY(cnt / 60.f) * XMMatrixTranslation(30.f, 0.f, 40.f - i * 40.f);
			map(&bunnyDataConstantBuffer, bunnyData);

			cnt++;



			auto backBufferIndex = swapChain.get_vcurrent_back_buffer_index();

			command.reset(backBufferIndex);


			//
			//ShadowMap
			//

			command.barrior(shadowMap, resource_state::RenderTarget);
			command.barrior(shadowMapDepthBuffer, resource_state::DepthWrite);

			command.clear_depth_view(shadowMapDSVDescriptorHeap.get_CPU_handle(), 1.f);
			command.clear_render_target_view(shadowMapRTVDescriptorHeap.get_CPU_handle(), { 1,1 });
			command.set_render_target({ {shadowMapRTVDescriptorHeap.get_CPU_handle()} }, shadowMapDSVDescriptorHeap.get_CPU_handle());
			command.set_viewport(shadowMapViewport);
			command.set_scissor_rect(shadowMapScissorRect);

			command.set_descriptor_heap(bunnyDescriptorHeap);
			command.set_graphics_root_signature(bunnyRootSignature);
			command.set_graphics_root_descriptor_table(0, bunnyDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(bunnyShadowMapPipelineState);
			command.set_vertex_buffer(bunnyVertexBuffer);
			command.set_index_buffer(bunnyIndexBuffer);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(bunnyIndexNum, BUNNY_NUM);

			command.set_descriptor_heap(groundDescriptorHeap);
			command.set_graphics_root_signature(groundRootSignature);
			command.set_graphics_root_descriptor_table(0, groundDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(groundShadowMapPipelineState);
			command.set_vertex_buffer(groundVertexBuffer);
			command.set_index_buffer(groundIndexBuffer);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(groundIndexNum);

			command.barrior(shadowMap, resource_state::PixcelShaderResource);


			//
			//GaussianBlur
			//

			command.barrior(gaussianBlurShadowMap, resource_state::UnorderedAccessResource);
			command.set_compute_root_signature(computeGaussianBlurRootSignature);
			command.set_descriptor_heap(computeGaussianBlurDescriptorHeap);
			command.set_compute_root_descriptor_table(0, computeGaussianBlurDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(computeGaussianBlurPipelineState);
			command.dispatch(SHADOW_MAP_EDGE / 8 + 1, SHADOW_MAP_EDGE / 8 + 1, 1);
			command.barrior(gaussianBlurShadowMap, resource_state::PixcelShaderResource);


			//
			//BackBuffer
			//

			command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
			command.barrior(depthBuffer, resource_state::DepthWrite);

			command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5,0.5,0.5,1.0 });
			command.clear_depth_view(dsvDescriptorHeap.get_CPU_handle(), 1.f);
			command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} }, dsvDescriptorHeap.get_CPU_handle());
			command.set_viewport(viewport);
			command.set_scissor_rect(scissorRect);

			command.set_descriptor_heap(bunnyDescriptorHeap);
			command.set_graphics_root_signature(bunnyRootSignature);
			command.set_graphics_root_descriptor_table(0, bunnyDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(bunnyPipelineState);
			command.set_vertex_buffer(bunnyVertexBuffer);
			command.set_index_buffer(bunnyIndexBuffer);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(bunnyIndexNum, BUNNY_NUM);

			command.set_descriptor_heap(groundDescriptorHeap);
			command.set_graphics_root_signature(groundRootSignature);
			command.set_graphics_root_descriptor_table(0, groundDescriptorHeap.get_GPU_handle());
			command.set_pipeline_state(groundPipelineState);
			command.set_vertex_buffer(groundVertexBuffer);
			command.set_index_buffer(groundIndexBuffer);
			command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
			command.draw_indexed_instanced(groundIndexNum);


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