#pragma once
#include"RootSignatureInitializeTag.hpp"
#include<array>
#include<vector>

#include"RootSignature.hpp"

namespace DX12
{
	enum class DescriptorRangeType;
	enum class StaticSamplerType;

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

	std::vector<D3D12_ROOT_PARAMETER> GetDescriptorTables(const std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>& ranges)
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

	D3D12_STATIC_SAMPLER_DESC GetStanderdStaticSampler(std::size_t registerNum) 
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

	D3D12_STATIC_SAMPLER_DESC GetToonStaticSampler(std::size_t registerNum)
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

	D3D12_STATIC_SAMPLER_DESC GetSadowMappingStaticSampler(std::size_t registerNum)
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

	std::vector<D3D12_STATIC_SAMPLER_DESC> GetStaticSamplers(const std::vector<StaticSamplerType>& staticSamplerTypes)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> result{};
		result.reserve(staticSamplerTypes.size());

		D3D12_STATIC_SAMPLER_DESC(*getStaticSamplerFuncs[])(std::size_t) = { GetStanderdStaticSampler ,GetToonStaticSampler,GetSadowMappingStaticSampler };

		for (std::size_t i = 0; i < staticSamplerTypes.size(); i++) {
			result.push_back(getStaticSamplerFuncs[static_cast<std::size_t>(staticSamplerTypes[i])](i));
		}

		return result;
	}


	//���W�X�^�̔ԍ����X�V����D3D12_DESCRIPTOR_RANGE���擾
	//4��DescriptorRangeTag�̎��
	template<typename DescriptorRangeType>
	constexpr auto GetDescriptorRange(std::array<std::size_t, 4>&& registerNums)
	{
		D3D12_DESCRIPTOR_RANGE descriptorRange{};
		descriptorRange.NumDescriptors = 1;
		descriptorRange.RangeType = DescriptorRangeType::value;
		descriptorRange.BaseShaderRegister = registerNums[DescriptorRangeType::value];
		descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		registerNums[DescriptorRangeType::value]++;

		return std::make_pair(std::move(descriptorRange), std::move(registerNums));
	}

	//�f�B�X�N���v�^�e�[�u���̎擾�̎���
	template<typename DescriptorTableTag, std::size_t Index, std::size_t N>
	constexpr auto GetDescriptorTableImpl(std::array<D3D12_DESCRIPTOR_RANGE, N>& result, std::array<std::size_t, 4>&& registerNums)
	{
		if constexpr (Index == std::tuple_size_v<typename DescriptorTableTag::Types>)
		{
			return std::move(registerNums);
		}
		else
		{
			auto [descriptorRange, returnNums] = GetDescriptorRange<std::tuple_element_t<Index, DescriptorTableTag::Types>>(std::move(registerNums));
			result[Index] = std::move(descriptorRange);
			return GetDescriptorTableImpl<DescriptorTableTag, Index + 1>(result, std::move(returnNums));
		}
	}

	//���W�X�^�̏����󂯎��f�B�X�N���v�^�e�[�u����Ԃ�
	template<typename DescriptorTableTag>
	constexpr auto GetDescriptorTable(std::array<std::size_t, 4>&& registerNums)
	{
		std::array<D3D12_DESCRIPTOR_RANGE, std::tuple_size_v<typename DescriptorTableTag::Types>> result{};
		auto returnNums = GetDescriptorTableImpl<DescriptorTableTag, 0>(result, std::move(registerNums));
		return std::make_pair(std::move(result), std::move(returnNums));
	}

	template<typename DescriptorTableArrayTag, std::size_t Index>
	constexpr auto GetDescriptorTableTupleImpl(std::array<std::size_t, 4>&& nums)
	{
		using NowDescriptorTableType = std::tuple_element_t<Index, typename DescriptorTableArrayTag::Types>;

		auto [rangeArray, returnNums] = GetDescriptorTable<NowDescriptorTableType>(std::move(nums));

		if constexpr (Index + 1 < std::tuple_size_v<typename DescriptorTableArrayTag::Types>)
		{
			auto lest = GetDescriptorTableTupleImpl<DescriptorTableArrayTag, Index + 1>(std::move(returnNums));
			return std::tuple_cat(std::make_tuple(std::move(rangeArray)), std::move(lest));
		}
		else
		{
			return std::make_tuple(std::move(rangeArray));
		}
	}

	//���[�g�p�����[�^�ɂ̓e�[�u���̃|�C���^��n���Ȃ���΂Ȃ�Ȃ��̂�
	//�ϐ��ɑ�������p�̃^�v�����R���p�C�����Ɏ擾����
	template<typename DescriptorTableArrayTag>
	constexpr auto GetDescriptorTableTuple()
	{
		return GetDescriptorTableTupleImpl<DescriptorTableArrayTag, 0>({});
	}

	template<std::size_t Index, typename DescriptorTableDataTupleType, typename ResultArray>
	void GetDescriptorTableStructArrayImpl(const DescriptorTableDataTupleType& descriptorTableArray, ResultArray& resultArray)
	{
		if constexpr (Index < std::tuple_size_v<DescriptorTableDataTupleType>)
		{
			D3D12_ROOT_PARAMETER descriptorTable{};
			descriptorTable.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			descriptorTable.DescriptorTable.pDescriptorRanges = std::get<Index>(descriptorTableArray).data();
			descriptorTable.DescriptorTable.NumDescriptorRanges = std::get<Index>(descriptorTableArray).size();
			descriptorTable.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			resultArray[Index] = std::move(descriptorTable);

			GetDescriptorTableStructArrayImpl<Index + 1>(descriptorTableArray, resultArray);
		}
	}

	//�ϐ��ɑ������ꂽ�f�B�X�N���v�^�e�[�u���̃^�v������A���̃|�C���^���擾��
	//���[�g�V�O�l�`���ɕK�v�ȍ\���̂��擾����
	template<typename DescriptorTableDataTupleType>
	auto GetDescriptorTableStructArray(const DescriptorTableDataTupleType& descriptorTableArray)
	{
		std::array<D3D12_ROOT_PARAMETER, std::tuple_size_v<DescriptorTableDataTupleType>> result{};
		GetDescriptorTableStructArrayImpl<0>(descriptorTableArray, result);
		return result;
	}

	template<typename StaticSamplerType, std::size_t Index>
	struct StaticSamplerDescGetter;

	template<std::size_t Index>
	struct StaticSamplerDescGetter<StaticSamplerTag::Standard, Index> {
		constexpr static auto Get() {

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
			result.RegisterSpace = Index;

			return result;
		}
	};

	template<std::size_t Index>
	struct StaticSamplerDescGetter<StaticSamplerTag::ShadowMapping, Index> {
		constexpr static auto Get() {

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
			result.RegisterSpace = Index;

			return result;
		}
	};

	template<std::size_t Index>
	struct StaticSamplerDescGetter<StaticSamplerTag::Toon, Index> {
		constexpr static auto Get() {

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
			result.RegisterSpace = Index;

			return result;
		}
	};

	template<typename StaticSamplersType, std::size_t Index>
	constexpr auto GetStaticSamplerArrayImpl(std::array<D3D12_STATIC_SAMPLER_DESC, std::tuple_size_v<typename StaticSamplersType::Types>>& resultArray)
	{
		if constexpr (Index < std::tuple_size_v<typename StaticSamplersType::Types>)
		{
			resultArray[Index] = StaticSamplerDescGetter<std::tuple_element_t<Index, typename StaticSamplersType::Types>, Index>::Get();
			GetStaticSamplerArrayImpl<StaticSamplersType, Index + 1>(resultArray);
		}
	}

	template<typename StaticSamplersType>
	constexpr auto GetStaticSamplerArray()
	{
		std::array<D3D12_STATIC_SAMPLER_DESC, std::tuple_size_v<typename StaticSamplersType::Types>> result{};
		GetStaticSamplerArrayImpl<StaticSamplersType, 0>(result);

		return result;
	}



	//
	//void�������ɂ������̏����p
	//

	//���O�ς�����
	struct VoidObject {
		constexpr std::size_t size() const noexcept {
			return 0;
		}

		constexpr auto data() const noexcept {
			return nullptr;
		}
	};

	template<>
	constexpr auto GetDescriptorTableTuple<void>()
	{
		return static_cast<int>(0);
	}

	auto GetDescriptorTableStructArray(int)
	{
		return VoidObject{};
	}

	template<>
	constexpr auto GetStaticSamplerArray<void>()
	{
		return VoidObject{};
	}


}