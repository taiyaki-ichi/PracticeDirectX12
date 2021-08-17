#pragma once
#include"Device.hpp"
#include"RootSignatureHelper.hpp"
#include<string>

namespace DX12
{

	enum class DescriptorRangeType {
		SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	};

	enum class StaticSamplerType {
		Standard,
		Toon,
		SadowMapping,
		Cubemap,
	};

	class RootSignature
	{
		ID3D12RootSignature* rootSignature = nullptr;

	public:
		RootSignature() = default;
		~RootSignature();

		RootSignature(const RootSignature&) = delete;
		RootSignature& operator=(const RootSignature&) = delete;

		RootSignature(RootSignature&&) noexcept;
		RootSignature& operator=(RootSignature&&) noexcept;

		void Initialize(Device*, const std::vector<std::vector<DescriptorRangeType>>&,const std::vector<StaticSamplerType>&);

		ID3D12RootSignature* Get();
	};


	//
	//
	//

	inline RootSignature::~RootSignature()
	{
		if (rootSignature)
			rootSignature->Release();
	}

	inline RootSignature::RootSignature(RootSignature&& rhs) noexcept
	{
		rootSignature = rhs.rootSignature;
		rhs.rootSignature = nullptr;
	}

	inline RootSignature& RootSignature::operator=(RootSignature&& rhs) noexcept
	{
		rootSignature = rhs.rootSignature;
		rhs.rootSignature = nullptr;

		return *this;
	}

	void RootSignature::Initialize(Device* device, const std::vector<std::vector<DescriptorRangeType>>& descriptorRangeTypes, 
		const std::vector<StaticSamplerType>& staticSamplerTypes)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		auto descriptorRanges = GetDescriptorRange(descriptorRangeTypes);
		auto descriptorTables = GetDescriptorTables(descriptorRanges);
		auto [descriptorTableData, descriptorTableSize] = GetContainerDataAndSize(descriptorTables);
		rootSignatureDesc.pParameters = descriptorTableData;
		rootSignatureDesc.NumParameters = descriptorTableSize;

		auto staticSamplers = GetStaticSamplers(staticSamplerTypes);
		auto [staticSamplerData, staticSamplerSize] = GetContainerDataAndSize(staticSamplers);
		rootSignatureDesc.pStaticSamplers = staticSamplerData;
		rootSignatureDesc.NumStaticSamplers = staticSamplerSize;
	

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		{
			auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
			if (FAILED(result)) {
				std::string errstr;
				errstr.resize(errorBlob->GetBufferSize());
				std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
				throw errstr + "\n";
			}
		}

		{
			auto result = device->Get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
			if (FAILED(result))
				throw "";

			rootSigBlob->Release();
		}
	}

	inline ID3D12RootSignature* RootSignature::Get()
	{
		return rootSignature;
	}

}