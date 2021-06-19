#pragma once
#include"RootSignature.hpp"
#include<array>
#include<vector>

namespace DX12
{
	enum class DescriptorRangeType;
	enum class StaticSamplerType;

	inline std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> GetDescriptorRange(const std::vector<std::vector<DescriptorRangeType>>& descriptorRangeTypes);

	inline std::vector<D3D12_ROOT_PARAMETER> GetDescriptorTables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges);

	inline D3D12_STATIC_SAMPLER_DESC GetStanderdStaticSampler(std::size_t registerNum);
	inline D3D12_STATIC_SAMPLER_DESC GetToonStaticSampler(std::size_t registerNum);
	inline D3D12_STATIC_SAMPLER_DESC GetSadowMappingStaticSampler(std::size_t registerNum);

	inline std::vector<D3D12_STATIC_SAMPLER_DESC> GetStaticSamplers(const std::vector<StaticSamplerType>& staticSamplerTypes);

	template<typename Container>
	std::pair<const typename Container::value_type*, std::size_t> GetContainerDataAndSize(const Container&);


	//
	//
	//


	inline std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> GetDescriptorRange(const std::vector<std::vector<DescriptorRangeType>>& descriptorRangeTypes) 
	{
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> result{};
		result.reserve(descriptorRangeTypes.size());

		std::array<std::size_t, 4> registerNums{};

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

	inline std::vector<D3D12_ROOT_PARAMETER> GetDescriptorTables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges)
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

	inline D3D12_STATIC_SAMPLER_DESC GetStanderdStaticSampler(std::size_t registerNum) 
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
		result.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
		result.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
		result.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�
		result.RegisterSpace = registerNum;

		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC GetToonStaticSampler(std::size_t registerNum)
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
		result.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;//��r���ʂ����j�A�⊮
		result.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
		result.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�
		result.MaxAnisotropy = 1;//�[�x�X�΂�L����
		result.RegisterSpace = registerNum;

		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC GetSadowMappingStaticSampler(std::size_t registerNum)
	{
		D3D12_STATIC_SAMPLER_DESC result{};
		result.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		result.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
		result.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
		result.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
		result.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
		result.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ�
		result.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�
		result.RegisterSpace = registerNum;

		return result;
	}

	inline D3D12_STATIC_SAMPLER_DESC GetCubemapStaticSampler(std::size_t registerNum)
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
		result.RegisterSpace = registerNum;

		return result;
	}

	inline std::vector<D3D12_STATIC_SAMPLER_DESC> GetStaticSamplers(const std::vector<StaticSamplerType>& staticSamplerTypes)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> result{};
		result.reserve(staticSamplerTypes.size());

		D3D12_STATIC_SAMPLER_DESC(*getStaticSamplerFuncs[])(std::size_t) = {
			GetStanderdStaticSampler ,GetToonStaticSampler,GetSadowMappingStaticSampler ,GetCubemapStaticSampler
		};

		for (std::size_t i = 0; i < staticSamplerTypes.size(); i++) {
			result.push_back(getStaticSamplerFuncs[static_cast<std::size_t>(staticSamplerTypes[i])](i));
		}

		return result;
	}

	template<typename Container>
	std::pair<const typename Container::value_type*, std::size_t> GetContainerDataAndSize(const Container& c) {
		if (c.size() > 0)
			return { c.data(),c.size() };
		else
			return { nullptr,0 };
	}

}