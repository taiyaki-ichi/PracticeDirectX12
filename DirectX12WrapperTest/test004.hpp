#pragma once
#include"window.hpp"
#include"device.hpp"
#include"pipeline_state.hpp"
#include"command.hpp"
#include"swap_chain.hpp"
#include"OffLoader.hpp"
#include"utility.hpp"
#include"resource/buffer_resource.hpp"
#include"resource/allow_render_target_texture_resource.hpp"
#include"resource/allow_depth_stencil_texture_resource.hpp"

#include"resource/map_stream.hpp"

#include<array>
#include<DirectXMath.h>

//鏡の表現
namespace test004
{
	using namespace DX12;
	using namespace DirectX;

	constexpr std::size_t WINDOW_WIDTH = 1024;
	constexpr std::size_t WINDOW_HEIGHT = 768;

	struct SceneData {
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT4 lightDir;
		XMFLOAT3 eye;
	};

	struct ColorObjectData {
		XMMATRIX world;
		std::array<float, 4> color;
	};

	constexpr std::size_t CUBE_MAP_EDGE = 512;

	struct CubemapSceneData {
		XMMATRIX view[6];
		XMMATRIX proj;
	};

	constexpr std::size_t FRAME_LATENCY_NUM = 2;

	using FrameBufferFormat = format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>;

	using MeshVertexFormatTuple = format_tuple<format<component_type::FLOAT, 32, 3>, format<component_type::FLOAT, 32, 3>>;

	class Mesh
	{
		std::size_t faceNum{};

		buffer_resource<MeshVertexFormatTuple,resource_heap_property::UPLOAD> vertexBuffer{};
		buffer_resource<format<component_type::UINT, 32, 1>,resource_heap_property::UPLOAD> indexBuffer{};


	public:
		void initialize(device& device,const char* fileName)
		{
			auto [vertexList, faceList] = OffLoader::LoadTriangularMeshFromOffFile<std::array<float, 3>, std::array<std::uint32_t, 3>>(fileName);
			auto normalList = GetVertexNormal(vertexList, faceList);

			vertexBuffer.initialize(device, vertexList.size());

			auto vertexMapStream = map(vertexBuffer);
			XMFLOAT3 tmpFloat3;
			for (std::size_t i = 0; i < vertexList.size(); i++)
			{
				for (std::size_t j = 0; j < 3; j++)
					vertexMapStream << vertexList[i][j];

				XMStoreFloat3(&tmpFloat3, normalList[i]);
				vertexMapStream << tmpFloat3.x << tmpFloat3.y << tmpFloat3.z;
			}


			indexBuffer.initialize(device, faceList.size() * 3);

			auto indexMapStream = map(indexBuffer);
			for (std::size_t i = 0; i < faceList.size(); i++)
				for (std::size_t j = 0; j < 3; j++)
					indexMapStream << faceList[i][j];

			faceNum = faceList.size();
		}

		void SetMesh(command<FRAME_LATENCY_NUM>& cl)
		{
			cl.set_vertex_buffer(vertexBuffer);
			cl.set_index_buffer(indexBuffer);
		}

		void Draw(command<FRAME_LATENCY_NUM>& cl)
		{
			cl.draw_indexed_instanced(faceNum * 3);
		}
	};

	template<std::size_t ColorObjectNum>
	class ColorObjectModel
	{
		std::array<buffer_resource<ColorObjectData,resource_heap_property::UPLOAD>, ColorObjectNum> colorBunnyDataConstantBuffer{};
		descriptor_heap_CBV_SRV_UAV descriptorHeap{};

	public:
		void initialize(device& device, buffer_resource<SceneData,resource_heap_property::UPLOAD>& sceneDataConstantBuffer, buffer_resource<CubemapSceneData,resource_heap_property::UPLOAD>& cubemapSceneDataConstantBuffer)
		{
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				colorBunnyDataConstantBuffer[i].initialize(device, 1);

			descriptorHeap.initialize(device, 2 + ColorObjectNum);
			descriptorHeap.push_back_CBV(device, sceneDataConstantBuffer);
			for (std::size_t i = 0; i < ColorObjectNum; i++)
				descriptorHeap.push_back_CBV(device, colorBunnyDataConstantBuffer[i]);
			descriptorHeap.push_back_CBV(device, cubemapSceneDataConstantBuffer);
		}

		template<typename T>
		void MapColorBunnyData(T&& t, std::size_t i)
		{
			auto tmp = map(colorBunnyDataConstantBuffer[i]);
			tmp << std::forward<T>(t);
		}

		void SetDescriptorHeap(command<FRAME_LATENCY_NUM>& cl, std::size_t i)
		{
			cl.set_descriptor_heap(descriptorHeap);
			cl.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle(0));
			cl.set_graphics_root_descriptor_table(1, descriptorHeap.get_GPU_handle(1 + i));
			cl.set_graphics_root_descriptor_table(2, descriptorHeap.get_GPU_handle(1 + ColorObjectNum));
		}
	};

	class ColorObjectRenderer
	{
		root_signature rootSignature{};
		graphics_pipeline_state<MeshVertexFormatTuple, format_tuple<FrameBufferFormat>> standerdPipelineState{};
		graphics_pipeline_state<MeshVertexFormatTuple, format_tuple<FrameBufferFormat>> cubemapPipelineState{};

	public:
		void initialize(device& device)
		{
			rootSignature.initialize(device,
				{ {descriptor_range_type::CBV},{descriptor_range_type::CBV }, { descriptor_range_type::CBV } },
				{}
			);

			{
				shader vs{};
				vs.initialize(L"Shader/ColorObject/VertexShader.hlsl", "main", "vs_5_0");

				shader ps{};
				ps.initialize(L"Shader/ColorObject/PixelShader.hlsl", "main", "ps_5_0");

				standerdPipelineState.initialize(device, rootSignature, { &vs, &ps },
					{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
				);
			}

			{
				shader vs{};
				vs.initialize(L"Shader/ColorObjectForCubemap/VertexShader.hlsl", "main", "vs_5_0");

				shader gs{};
				gs.initialize(L"Shader/ColorObjectForCubemap/GeometryShader.hlsl", "main", "gs_5_0");

				shader ps{};
				ps.initialize(L"Shader/ColorObjectForCubemap/PixelShader.hlsl", "main", "ps_5_0");

				cubemapPipelineState.initialize(device, rootSignature, { &vs, &ps,&gs },
					{ "POSITION","NORMAL" }, true, false, primitive_topology::TRIANGLE
				);
			}
		}

		auto& GetStanderdPipelineState() noexcept {
			return standerdPipelineState;
		}
		auto& GetCubemapPipelineState() noexcept {
			return cubemapPipelineState;
		}
		root_signature& GetRootSignature() noexcept {
			return rootSignature;
		}

	};

	class MirrorObjectModel
	{
		buffer_resource<XMMATRIX,resource_heap_property::UPLOAD> worldConstantBuffer{};
		allow_render_target_texture_2D_resource<format<component_type::UNSIGNED_NORMALIZE_FLOAT, 8, 4>> cubemapShaderResource{};

		descriptor_heap_CBV_SRV_UAV descriptorHeap{};

	public:
		static constexpr std::array<float, 4> CUBEMAP_CLEAR_VALUE{ 0.5f,0.5f,0.5f,1.f };

		void initialize(device& device, buffer_resource<SceneData,resource_heap_property::UPLOAD>& sceneConstantBufferResource)
		{
			worldConstantBuffer.initialize(device, 1);

			cubemapShaderResource.initialize(device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, 6, 1, { {0.5f,0.5f,0.5f,1.f} });

			descriptorHeap.initialize(device, 3);
			descriptorHeap.push_back_CBV(device, sceneConstantBufferResource);
			descriptorHeap.push_back_CBV(device, worldConstantBuffer);
			descriptorHeap.push_back_texture_cube_array_SRV(device, cubemapShaderResource, 6, 0, 1, 0, 0, 0.f);
		}

		template<typename T>
		void MapWorld(T&& t) {
			auto tmp = map(worldConstantBuffer);
			tmp << std::forward<T>(t);
		}

		auto& GetCubemapShaderResource() {
			return cubemapShaderResource;
		}

		void SetDescriptorHeap(command<FRAME_LATENCY_NUM>& cl)
		{
			cl.set_descriptor_heap(descriptorHeap);
			cl.set_graphics_root_descriptor_table(0, descriptorHeap.get_GPU_handle());
		}
	};


	class MirrorObjectRenderer
	{
		root_signature rootSignature{};
		graphics_pipeline_state<MeshVertexFormatTuple, format_tuple<FrameBufferFormat>> pipelineState{};

	public:
		void initialize(device& device)
		{

			rootSignature.initialize(device,
				{ {descriptor_range_type::CBV,descriptor_range_type::CBV , descriptor_range_type::SRV } },
				{ static_sampler_desc::clamp_anisotropic() }
			);

			shader vs{};
			vs.initialize(L"Shader/MirrorObject/VertexShader.hlsl", "main", "vs_5_0");

			shader ps{};
			ps.initialize(L"Shader/MirrorObject/PixelShader.hlsl", "main", "ps_5_0");

			pipelineState.initialize(device, rootSignature, { &vs, &ps },
				{ "POSITION", "NORMAL" }, true, false, primitive_topology::TRIANGLE
			);
		}

		auto& GetPipelineState() noexcept {
			return pipelineState;
		}
		auto& GetRootSignature() noexcept {
			return rootSignature;
		}
	};


	inline CubemapSceneData GetCubemapSceneData(const XMFLOAT3& pos)
	{
		CubemapSceneData result;

		std::array<XMFLOAT3, 6> up{
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,0.f,-1.f),
			XMFLOAT3(0.f,0.f,1.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
		};
		std::array<XMFLOAT3, 6> target{
			XMFLOAT3(1.f,0.f,0.f),
			XMFLOAT3(-1.f,0.f,0.f),
			XMFLOAT3(0.f,1.f,0.f),
			XMFLOAT3(0.f,-1.f,0.f),
			XMFLOAT3(0.f,0.f,1.f),
			XMFLOAT3(0.f,0.f,-1.f),
		};

		for (std::size_t i = 0; i < 6; i++)
			result.view[i] = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target[i]), XMLoadFloat3(&up[i]));

		result.proj = XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2,
			static_cast<float>(CUBE_MAP_EDGE) / static_cast<float>(CUBE_MAP_EDGE), 0.1f, 100.f);

		return result;
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


		buffer_resource<SceneData,resource_heap_property::UPLOAD> sceneDataConstantBuffer{};
		sceneDataConstantBuffer.initialize(device, 1);

		buffer_resource<CubemapSceneData,resource_heap_property::UPLOAD> cubemapSceneDataConstant{};
		cubemapSceneDataConstant.initialize(device, 1);


		Mesh bunnyMesh{};
		bunnyMesh.initialize(device, "../../Assets/bun_zipper_res3.off");

		Mesh sphereMesh{};
		sphereMesh.initialize(device, "../../Assets/sphere.off");


		constexpr std::size_t ColorObjectNum = 4;

		std::array<XMMATRIX, ColorObjectNum> colorObjectWorlds{
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(10.f,0.f,0.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(-9.f,0.f,0.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(0.f, 0.f, 7.f),
			XMMatrixScaling(20.f, 20.f, 20.f) * XMMatrixTranslation(0.f, 0.f, -6.f)
		};

		std::array<std::array<float, 4>, ColorObjectNum> colorObjectColors{ {
			{1.f, 0.f, 0.f, 1.f},
			{0.f, 1.f, 0.f, 1.f},
			{0.f, 0.f, 1.f, 1.f},
			{1.f, 1.f, 0.f, 1.f}
		} };

		ColorObjectModel<ColorObjectNum> colorObjectModel{};
		colorObjectModel.initialize(device, sceneDataConstantBuffer, cubemapSceneDataConstant);

		ColorObjectRenderer colorObjectRenderer{};
		colorObjectRenderer.initialize(device);


		MirrorObjectModel mirrorObjectModel{};
		mirrorObjectModel.initialize(device, sceneDataConstantBuffer);

		MirrorObjectRenderer mirrorObjectRenderer{};
		mirrorObjectRenderer.initialize(device);

		allow_depth_stencil_texture_2D_resource<format<component_type::FLOAT, 32, 1>> cubemapDepthBuffer{};
		cubemapDepthBuffer.initialize(device, CUBE_MAP_EDGE, CUBE_MAP_EDGE, 6, 1, 1.f, 0);

		descriptor_heap_RTV cubemapRtvDescriptorHeap{};
		cubemapRtvDescriptorHeap.initialize(device, 1);
		cubemapRtvDescriptorHeap.push_back_texture2D_array_RTV(device, mirrorObjectModel.GetCubemapShaderResource(), 6, 0, 0, 0);


		allow_depth_stencil_texture_2D_resource<format<component_type::FLOAT, 32, 1>> depthBuffer{};
		depthBuffer.initialize(device, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1, 1.f, 0);

		descriptor_heap_DSV depthStencilDescriptorHeap{};
		depthStencilDescriptorHeap.initialize(device, 2);
		depthStencilDescriptorHeap.push_back_texture2D_DSV(device, depthBuffer, 0);
		//ここにViewを作ってしまおう
		depthStencilDescriptorHeap.push_back_texture2D_array_DSV(device, cubemapDepthBuffer, 6, 0, 0);
	

		D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
		D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

		D3D12_VIEWPORT cubemapViewport{ 0,0, static_cast<float>(CUBE_MAP_EDGE),static_cast<float>(CUBE_MAP_EDGE),0.f,1.f };
		D3D12_RECT cubemapScissorRect{ 0,0,static_cast<LONG>(CUBE_MAP_EDGE),static_cast<LONG>(CUBE_MAP_EDGE) };

		float len = 4.f;
		XMFLOAT3 eye{ len,5,len };
		XMFLOAT3 target{ 0,0,0 };
		XMFLOAT3 up{ 0,1,0 };
		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		auto proj = XMMatrixPerspectiveFovLH(
			DirectX::XM_PIDIV2,
			static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
			0.1f,
			100.f
		);
		XMFLOAT4 lightDir{ 1.f,1.f,1.f,1.f };
		XMFLOAT3 mirrorPos{ 0.f, 0.f, 0.f };

		auto sceneDataStream = map(sceneDataConstantBuffer);
		sceneDataStream << SceneData{ view,proj,lightDir,eye };

		while (update_window())
		{
			for (std::size_t i = 0; i < ColorObjectNum; i++) {
				colorObjectWorlds[i] *= XMMatrixRotationY(0.02);
				colorObjectModel.MapColorBunnyData(ColorObjectData{ colorObjectWorlds[i] ,colorObjectColors[i] }, i);
			}

			/*
			*cubemapSceneDataMappedResource.begin() = GetCubemapSceneData(mirrorPos);
			* */
			auto cubemapSceneDataStream = map(cubemapSceneDataConstant);
			cubemapSceneDataStream << GetCubemapSceneData(mirrorPos);

			mirrorObjectModel.MapWorld(XMMatrixScaling(3.f, 3.f, 3.f) * XMMatrixTranslation(mirrorPos.x, mirrorPos.y, mirrorPos.z));


			auto backBufferIndex = swapChain.get_current_back_buffer_index();
			command.reset(backBufferIndex);

			//Cubemap
			{
				command.set_viewport(cubemapViewport);
				command.set_scissor_rect(cubemapScissorRect);

				command.barrior(mirrorObjectModel.GetCubemapShaderResource(), resource_state::RenderTarget);
				command.barrior(cubemapDepthBuffer, resource_state::DepthWrite);

				command.clear_render_target_view(cubemapRtvDescriptorHeap.get_CPU_handle(), MirrorObjectModel::CUBEMAP_CLEAR_VALUE);
				command.clear_depth_view(depthStencilDescriptorHeap.get_CPU_handle(1), 1.f);

				command.set_render_target({ {cubemapRtvDescriptorHeap.get_CPU_handle()} }, depthStencilDescriptorHeap.get_CPU_handle(1));

				command.set_pipeline_state(colorObjectRenderer.GetCubemapPipelineState());
				command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
				command.set_graphics_root_signature(colorObjectRenderer.GetRootSignature());
				bunnyMesh.SetMesh(command);

				for (std::size_t i = 0; i < ColorObjectNum; i++) {
					colorObjectModel.SetDescriptorHeap(command, i);
					bunnyMesh.Draw(command);
				}

				command.barrior(mirrorObjectModel.GetCubemapShaderResource(), resource_state::PixcelShaderResource);
			}

			//BackBuffer
			{
				command.set_viewport(viewport);
				command.set_scissor_rect(scissorRect);

				command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::RenderTarget);
				command.clear_render_target_view(rtvDescriptorHeap.get_CPU_handle(backBufferIndex), { 0.5f,0.5f,0.5f,1.f });

				command.barrior(depthBuffer, resource_state::DepthWrite);
				command.clear_depth_view(depthStencilDescriptorHeap.get_CPU_handle(), 1.f);

				command.set_render_target({ {rtvDescriptorHeap.get_CPU_handle(backBufferIndex)} }, depthStencilDescriptorHeap.get_CPU_handle());


				//ColorBunny
				{
					command.set_pipeline_state(colorObjectRenderer.GetStanderdPipelineState());
					command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
					command.set_graphics_root_signature(colorObjectRenderer.GetRootSignature());
					bunnyMesh.SetMesh(command);

					for (std::size_t i = 0; i < ColorObjectNum; i++) {
						colorObjectModel.SetDescriptorHeap(command, i);
						bunnyMesh.Draw(command);
					}
				}

				//MirrorSphere
				{
					command.set_pipeline_state(mirrorObjectRenderer.GetPipelineState());
					command.set_primitive_topology(primitive_topology::TRIANGLE_LIST);
					command.set_graphics_root_signature(mirrorObjectRenderer.GetRootSignature());
					mirrorObjectModel.SetDescriptorHeap(command);
					sphereMesh.SetMesh(command);
					sphereMesh.Draw(command);
				}

				command.barrior(swapChain.get_frame_buffer(backBufferIndex), resource_state::Common);
			}


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