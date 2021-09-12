#pragma once
#include"device.hpp"
#include<string>
#include<vector>

namespace DX12
{

	enum class descriptor_range_type {
		SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	};

	//テクスチャ座標が0-1に収まっていないときの挙動
	enum class address_mode
	{
		WRAP = D3D12_TEXTURE_ADDRESS_MODE_WRAP,//繰り返し
		MIRROR = D3D12_TEXTURE_ADDRESS_MODE_MIRROR,//反転
		CLAMP = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,//0または1のテクスチャカラーに
		BORDER = D3D12_TEXTURE_ADDRESS_MODE_BORDER,//sampler_descで指定する境界色に
		MIRROR_ONECE = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE,//MIRRORしCLAMPする
	};

	enum class filter_type
	{
		POINT,//特に補完しない
		LINEAR,//線形補完
		ANISOTROPIC,//異方性、これが1番キレイ
	};

	enum class comparison_type
	{
		NONE,//ナシ
		LESS = D3D12_COMPARISON_FUNC_LESS,
		EQUAL = D3D12_COMPARISON_FUNC_EQUAL,
		LESS_EQUAL = D3D12_COMPARISON_FUNC_LESS_EQUAL,
		GREATER = D3D12_COMPARISON_FUNC_GREATER,
		NOT_EQUAL = D3D12_COMPARISON_FUNC_NOT_EQUAL,
		GREATER_EQUAL = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
	};

	struct static_sampler_desc
	{
		filter_type filter;
		address_mode address_u;
		address_mode address_v;
		address_mode address_w;
		comparison_type comparison;


		std::uint32_t max_anisotropy = 16;
		float min_LOD = 0.f;
		float max_LOD = D3D12_FLOAT32_MAX;
		float mip_LOD_bias = 0;

		

		static constexpr static_sampler_desc clamp_point();
		static constexpr static_sampler_desc clamp_liner();
		static constexpr static_sampler_desc clamp_anisotropic();
		static constexpr static_sampler_desc wrap_point();
		static constexpr static_sampler_desc wrap_liner();
		static constexpr static_sampler_desc wrap_anisotropic();
	};



	class root_signature
	{
		release_unique_ptr<ID3D12RootSignature> root_signature_ptr{};

	public:
		root_signature() = default;
		~root_signature() = default;

		root_signature(root_signature&&) = default;
		root_signature& operator=(root_signature&&) = default;

		void initialize(device&, const std::vector<std::vector<descriptor_range_type>>&, const std::vector<static_sampler_desc>&);

		ID3D12RootSignature* Get();
	};


	//ヘルパ
	inline std::vector<D3D12_STATIC_SAMPLER_DESC> get_static_samplers(const std::vector<static_sampler_desc>& staticSamplerDescs);
	inline std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> get_descriptor_range(const std::vector<std::vector<descriptor_range_type>>& descriptorRangeTypes);
	inline std::vector<D3D12_ROOT_PARAMETER> get_descriptor_tables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges);

	//
	//
	//

	constexpr static_sampler_desc DX12::static_sampler_desc::clamp_point()
	{
		return { filter_type::POINT,address_mode::CLAMP,address_mode::CLAMP,address_mode::CLAMP,comparison_type::NONE };
	}

	inline constexpr static_sampler_desc DX12::static_sampler_desc::clamp_liner()
	{
		return { filter_type::LINEAR,address_mode::CLAMP,address_mode::CLAMP,address_mode::CLAMP,comparison_type::NONE };
	}

	inline constexpr static_sampler_desc DX12::static_sampler_desc::clamp_anisotropic()
	{
		return { filter_type::ANISOTROPIC,address_mode::CLAMP,address_mode::CLAMP,address_mode::CLAMP,comparison_type::NONE };
	}

	constexpr static_sampler_desc DX12::static_sampler_desc::wrap_point()
	{
		return { filter_type::POINT,address_mode::WRAP,address_mode::WRAP,address_mode::WRAP,comparison_type::NONE };
	}

	inline constexpr static_sampler_desc DX12::static_sampler_desc::wrap_liner()
	{
		return { filter_type::LINEAR,address_mode::WRAP,address_mode::WRAP,address_mode::WRAP,comparison_type::NONE };
	}

	inline constexpr static_sampler_desc DX12::static_sampler_desc::wrap_anisotropic()
	{
		return { filter_type::ANISOTROPIC,address_mode::WRAP,address_mode::WRAP,address_mode::WRAP,comparison_type::NONE };
	}

	void root_signature::initialize(device& device, const std::vector<std::vector<descriptor_range_type>>& descriptorRangeTypes,
		const std::vector<static_sampler_desc>& staticSamplerDescs)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		auto descriptorRanges = get_descriptor_range(descriptorRangeTypes);
		auto descriptorTables = get_descriptor_tables(descriptorRanges);
		rootSignatureDesc.NumParameters = descriptorTables.size();
		rootSignatureDesc.pParameters = descriptorTables.size() > 0 ? descriptorTables.data() : nullptr;

		auto staticSamplers = get_static_samplers(staticSamplerDescs);
		rootSignatureDesc.NumStaticSamplers = staticSamplers.size();
		rootSignatureDesc.pStaticSamplers = staticSamplers.size() > 0 ? staticSamplers.data() : nullptr;

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		{
			auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
			if (FAILED(result)) {
				std::string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				THROW_EXCEPTION(errstr.data());
			}
		}

		{
			ID3D12RootSignature* tmp = nullptr;
			auto result = device.get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&tmp));
			if (FAILED(result))
				THROW_EXCEPTION("");
			root_signature_ptr.reset(tmp);

			rootSigBlob->Release();
		}
	}

	inline ID3D12RootSignature* root_signature::Get()
	{
		return root_signature_ptr.get();
	}


	inline std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> get_descriptor_range(const std::vector<std::vector<descriptor_range_type>>& descriptorRangeTypes)
	{
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> result{};
		result.reserve(descriptorRangeTypes.size());

		std::array<std::uint32_t, 4> registerNums{};

		for (auto& rangeTypes : descriptorRangeTypes) {
			std::vector<D3D12_DESCRIPTOR_RANGE> ranges{};
			ranges.reserve(rangeTypes.size());

			for (auto& t : rangeTypes) {
				D3D12_DESCRIPTOR_RANGE descriptorRange{};
				descriptorRange.NumDescriptors = 1;
				descriptorRange.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t);
				descriptorRange.BaseShaderRegister = registerNums[static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t)];
				descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				ranges.push_back(std::move(descriptorRange));
				registerNums[static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t)]++;
			}

			result.push_back(std::move(ranges));
		}

		return result;
	}

	inline std::vector<D3D12_ROOT_PARAMETER> get_descriptor_tables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges)
	{
		std::vector<D3D12_ROOT_PARAMETER> result{};
		result.reserve(ranges.size());

		for (auto& r : ranges) {
			D3D12_ROOT_PARAMETER descriptorTable{};
			descriptorTable.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			descriptorTable.DescriptorTable.pDescriptorRanges = r.data();
			descriptorTable.DescriptorTable.NumDescriptorRanges = r.size();
			descriptorTable.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			result.push_back(std::move(descriptorTable));
		}

		return result;
	}


	inline std::vector<D3D12_STATIC_SAMPLER_DESC> get_static_samplers(const std::vector<static_sampler_desc>& staticSamplerDescs)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> result(staticSamplerDescs.size());

		for (std::uint32_t i = 0; i < staticSamplerDescs.size(); i++)
		{
			result[i].AddressU = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(staticSamplerDescs[i].address_u);
			result[i].AddressV = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(staticSamplerDescs[i].address_v);
			result[i].AddressW = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(staticSamplerDescs[i].address_w);
			result[i].ComparisonFunc = static_cast<D3D12_COMPARISON_FUNC>(staticSamplerDescs[i].comparison);

			if (staticSamplerDescs[i].comparison == comparison_type::NONE)
			{
				switch (staticSamplerDescs[i].filter)
				{
				case filter_type::POINT:
					result[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
					break;
				case filter_type::LINEAR:
					result[i].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
					break;
				case filter_type::ANISOTROPIC:
					result[i].Filter = D3D12_FILTER_ANISOTROPIC;
					break;
				}
			}
			else
			{
				switch (staticSamplerDescs[i].filter)
				{
				case filter_type::POINT:
					result[i].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
					break;
				case filter_type::LINEAR:
					result[i].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
					break;
				case filter_type::ANISOTROPIC:
					result[i].Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
					break;
				}
			}
			
			result[i].MaxAnisotropy = staticSamplerDescs[i].max_anisotropy;

			result[i].MinLOD = staticSamplerDescs[i].min_LOD;
			result[i].MaxLOD = staticSamplerDescs[i].max_LOD;

			result[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			result[i].ShaderRegister = i;
		}

		return result;
	}
}