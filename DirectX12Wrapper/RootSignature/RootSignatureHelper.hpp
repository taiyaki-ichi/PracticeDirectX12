#pragma once
#include"RootSignature.hpp"
#include<array>
#include<vector>

namespace DX12
{
	enum class descriptor_range_type;
	enum class StaticSamplerType;

	inline std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> get_descriptor_range(const std::vector<std::vector<descriptor_range_type>>& descriptorRangeTypes);

	inline std::vector<D3D12_ROOT_PARAMETER> get_descriptor_tables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges);

	inline D3D12_STATIC_SAMPLER_DESC get_standerd_static_sampler(std::uint32_t registerNum);
	inline D3D12_STATIC_SAMPLER_DESC get_toon_static_sampler(std::uint32_t registerNum);
	inline D3D12_STATIC_SAMPLER_DESC get_sadow_mapping_static_sampler(std::uint32_t registerNum);

	inline std::vector<D3D12_STATIC_SAMPLER_DESC> get_static_samplers(const std::vector<StaticSamplerType>& staticSamplerTypes);

	template<typename Container>
	std::pair<const typename Container::value_type*, std::uint32_t> get_container_data_and_size(const Container&);


	//
	//
	//


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

	inline D3D12_STATIC_SAMPLER_DESC get_standerd_static_sampler(std::uint32_t registerNum) 
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
		result.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
		result.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
		result.MinLOD = 0.0f;//ミップマップ最小値
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		result.ShaderRegister = registerNum;

		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC get_toon_static_sampler(std::uint32_t registerNum)
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
		result.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;//比較結果をリニア補完
		result.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
		result.MinLOD = 0.0f;//ミップマップ最小値
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//result.MaxAnisotropy = 1;//深度傾斜を有効に
		result.ShaderRegister = registerNum;

		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC get_sadow_mapping_static_sampler(std::uint32_t registerNum)
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
		result.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
		result.MinLOD = 0.0f;//ミップマップ最小値
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		result.ShaderRegister = registerNum;

		result.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		result.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;//比較結果をリニア補完
	
		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC GetCubemapStaticSampler(std::uint32_t registerNum)
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.Filter = D3D12_FILTER_ANISOTROPIC;
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		result.MinLOD = 0;
		result.MaxAnisotropy = 16;
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		result.MinLOD = 0.f;
		result.MaxLOD = D3D12_FLOAT32_MAX;
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		result.ShaderRegister = registerNum;

		return result;
	}

	inline std::vector<D3D12_STATIC_SAMPLER_DESC> get_static_samplers(const std::vector<StaticSamplerType>& staticSamplerTypes)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> result{};
		result.reserve(staticSamplerTypes.size());

		D3D12_STATIC_SAMPLER_DESC(*getStaticSamplerFuncs[])(std::uint32_t) = {
			get_standerd_static_sampler ,get_toon_static_sampler,get_sadow_mapping_static_sampler ,GetCubemapStaticSampler
		};

		for (std::uint32_t i = 0; i < staticSamplerTypes.size(); i++) {
			result.push_back(getStaticSamplerFuncs[static_cast<std::uint32_t>(staticSamplerTypes[i])](i));
		}

		return result;
	}

	template<typename Container>
	std::pair<const typename Container::value_type*, std::uint32_t> get_container_data_and_size(const Container& c) {
		if (c.size() > 0)
			return { c.data(),c.size() };
		else
			return { nullptr,0 };
	}

}