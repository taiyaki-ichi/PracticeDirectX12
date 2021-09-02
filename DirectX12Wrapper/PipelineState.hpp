#pragma once
#include"RootSignature/RootSignature.hpp"
#include"Format.hpp"
#include"Shader.hpp"
#include"Device.hpp"
#include"PrimitiveTopology.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	enum class semantics
	{
		POSITION,
		NORMAl,
		TEXDOORD,
	};

	template<component_type Type,std::uint32_t Size,std::uint32_t Num>
	struct converte_format {
		//static_assert(false);
		using type;
	};

	template<std::uint32_t Num>
	struct converte_format<component_type::FLOAT, 32, Num> {
		using type = std::array<float, Num>;
	};

	template<std::uint32_t Num>
	struct converte_format<component_type::UINT, 32, Num> {
		using type = std::array<std::uint32_t, Num>;
	};

	template<std::uint32_t Num>
	struct converte_format<component_type::UINT, 16, Num> {
		using type = std::array<std::uint16_t, Num>;
	};

	template<std::uint32_t Num>
	struct converte_format<component_type::UINT, 8, Num> {
		using type = std::array<std::uint8_t, Num>;
	};

	template<std::size_t Num,typename... Formats>
	struct vertex_layout_format_type {
		using type = std::tuple<typename converte_format<Formats::component_type, Formats::component_size, Formats::component_num>::type...>;
	};

	template<typename Formats>
	struct vertex_layout_format_type<1,Formats> {
		using type = typename converte_format<Formats::component_type, Formats::component_size, Formats::component_num>::type;
	};

	template<typename... Formats>
	struct vertex_layout
	{
		constexpr static std::size_t format_num = sizeof...(Formats);
		using format_tuple = std::tuple<Formats...>;
		using struct_type = typename vertex_layout_format_type<sizeof...(Formats), Formats...>::type;
		using resource_type = vertex_buffer_resource<Formats...>;
	};

	template<typename... Formats>
	struct render_target_formats 
	{
		constexpr static std::size_t format_num = sizeof...(Formats);
		using format_tuple = std::tuple<Formats...>;
	};

	struct ShaderDesc {
		Shader* vertexShader = nullptr;
		Shader* pixcelShader = nullptr;
		Shader* geometryShader = nullptr;
		Shader* hullShader = nullptr;
		Shader* domainShader = nullptr;
	};

	template<std::size_t I, typename VertexLayout>
	inline constexpr void get_input_element_desc_impl(std::array<const char*, VertexLayout::format_num>& vertexName,
		std::array<D3D12_INPUT_ELEMENT_DESC, VertexLayout::format_num>& result)
	{
		result[I].SemanticName = vertexName[I];
		result[I].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		result[I].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		using Format = std::remove_reference_t<decltype(std::get<I>(std::declval<typename VertexLayout::format_tuple>()))>;
		result[I].Format = get_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();

		if constexpr (I + 1 < VertexLayout::format_num)
			get_input_element_desc_impl<I + 1, VertexLayout>(vertexName, result);
	}
	
	template<typename VertexLayout>
	inline constexpr std::array<D3D12_INPUT_ELEMENT_DESC, VertexLayout::format_num> get_input_element_desc(std::array<const char*, VertexLayout::format_num>& vertexName)
	{
		std::array<D3D12_INPUT_ELEMENT_DESC, VertexLayout::format_num> result{};
		get_input_element_desc_impl<0, VertexLayout>(vertexName, result);
		return result;
	}

	template<std::size_t I, typename RenderTargetFormats>
	inline constexpr void get_render_target_format(DXGI_FORMAT (&result)[8] )
	{
		using Format = std::remove_reference_t<decltype(std::get<I>(std::declval<typename RenderTargetFormats::format_tuple>()))>;
		result[I] = get_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();

		if constexpr (I + 1 < RenderTargetFormats::format_num)
			get_render_target_format<I + 1, RenderTargetFormats>(result);
	}



	template<typename VertexLayout,typename ResnderTargetFormats>
	class graphics_pipeline_state
	{
		release_unique_ptr<ID3D12PipelineState> pipeline_state_ptr{};

	public:
		graphics_pipeline_state() = default;
		~graphics_pipeline_state() = default;

		graphics_pipeline_state(graphics_pipeline_state&&) = default;
		graphics_pipeline_state& operator=(graphics_pipeline_state&&) = default;

		void Initialize(Device&, RootSignature&, ShaderDesc,
			std::array<const char*,VertexLayout::format_num> vertexName,
			bool depthEnable, bool alphaBlend, PrimitiveTopology primitiveTopology
		);

		ID3D12PipelineState* Get() const noexcept;

	};


	class compute_pipeline_state
	{
		release_unique_ptr<ID3D12PipelineState> pipeline_state_ptr{};

	public:
		compute_pipeline_state() = default;
		~compute_pipeline_state() = default;

		compute_pipeline_state(compute_pipeline_state&&) = default;
		compute_pipeline_state& operator=(compute_pipeline_state&&) = default;

		//コンピュートシェーダ用のパイプライン生成
		void Initialize(Device&, RootSignature&, Shader& computeShader);

		ID3D12PipelineState* Get() const noexcept;
	};


	//
	//
	//


	template<typename VertexLayout, typename ResnderTargetFormats>
	inline void graphics_pipeline_state<VertexLayout, ResnderTargetFormats>::Initialize(Device& device, RootSignature& rootSignature, ShaderDesc shaderDesc, 
		std::array<const char*, VertexLayout::format_num> vertexName, bool depthEnable, bool alphaBlend, PrimitiveTopology primitiveTopology)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		if (shaderDesc.vertexShader) {
			graphicsPipelineDesc.VS.pShaderBytecode = shaderDesc.vertexShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.VS.BytecodeLength = shaderDesc.vertexShader->Get()->GetBufferSize();
		}
		if (shaderDesc.pixcelShader) {
			graphicsPipelineDesc.PS.pShaderBytecode = shaderDesc.pixcelShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.PS.BytecodeLength = shaderDesc.pixcelShader->Get()->GetBufferSize();
		}
		if (shaderDesc.geometryShader) {
			graphicsPipelineDesc.GS.pShaderBytecode = shaderDesc.geometryShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.GS.BytecodeLength = shaderDesc.geometryShader->Get()->GetBufferSize();
		}
		if (shaderDesc.hullShader) {
			graphicsPipelineDesc.HS.pShaderBytecode = shaderDesc.hullShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.HS.BytecodeLength = shaderDesc.hullShader->Get()->GetBufferSize();
		}
		if (shaderDesc.domainShader) {
			graphicsPipelineDesc.DS.pShaderBytecode = shaderDesc.domainShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.DS.BytecodeLength = shaderDesc.domainShader->Get()->GetBufferSize();
		}

		auto inputElementDescs = get_input_element_desc<VertexLayout>(vertexName);

		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputElementDescs.data();
		graphicsPipelineDesc.InputLayout.NumElements = inputElementDescs.size();
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//カット値はナシ
		graphicsPipelineDesc.PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(primitiveTopology);


		//ラスタライザの設定
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		//カリングはしない。つまり面の向きが裏でも描写する
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		graphicsPipelineDesc.RasterizerState = rasterizerDesc;


		//ブレンドステート
		D3D12_RENDER_TARGET_BLEND_DESC renderTagetBlendDesc{};
		if (alphaBlend)
		{
			renderTagetBlendDesc.BlendEnable = TRUE;
			renderTagetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;;
			renderTagetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			renderTagetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			renderTagetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			renderTagetBlendDesc.LogicOpEnable = FALSE;
		}
		else {
			renderTagetBlendDesc.BlendEnable = FALSE;//とりあえず
			renderTagetBlendDesc.LogicOpEnable = FALSE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = TRUE;
		blendDesc.IndependentBlendEnable = FALSE;
		//renderTagetBlendDesc.BlendEnableがFalseなので1つ目のみ設定で十分
		blendDesc.RenderTarget[0] = renderTagetBlendDesc;

		graphicsPipelineDesc.BlendState = blendDesc;

		graphicsPipelineDesc.NumRenderTargets = ResnderTargetFormats::format_num;
		if constexpr (ResnderTargetFormats::format_num > 0)
			get_render_target_format<0, ResnderTargetFormats>(graphicsPipelineDesc.RTVFormats);


		//デプスステンシル
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = depthEnable;
		if (depthEnable) {
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		depthStencilDesc.StencilEnable = FALSE;

		graphicsPipelineDesc.DepthStencilState = depthStencilDesc;

		//サンプルディスク
		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//これないとまずい

		graphicsPipelineDesc.pRootSignature = rootSignature.Get();

		{
			ID3D12PipelineState* tmp = nullptr;
			if (FAILED(device.Get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			pipeline_state_ptr.reset(tmp);
		}
	}

	template<typename VertexLayout, typename ResnderTargetFormats>
	inline ID3D12PipelineState* graphics_pipeline_state<VertexLayout, ResnderTargetFormats>::Get() const noexcept
	{
		return pipeline_state_ptr.get();
	}

	inline void compute_pipeline_state::Initialize(Device& device, RootSignature& rootSignature, Shader& computeShader)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
		computePipelineDesc.CS.pShaderBytecode = computeShader.Get()->GetBufferPointer();
		computePipelineDesc.CS.BytecodeLength = computeShader.Get()->GetBufferSize();
		computePipelineDesc.pRootSignature = rootSignature.Get();

		{
			ID3D12PipelineState* tmp = nullptr;
			if (FAILED(device.Get()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			pipeline_state_ptr.reset(tmp);
		}
	}

	inline ID3D12PipelineState* compute_pipeline_state::Get() const noexcept
	{
		return pipeline_state_ptr.get();
	}
}