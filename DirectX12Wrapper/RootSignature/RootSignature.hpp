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
		release_unique_ptr<ID3D12RootSignature> root_signature_ptr{};

	public:
		RootSignature() = default;
		~RootSignature() = default;

		RootSignature(RootSignature&&) = default;
		RootSignature& operator=(RootSignature&&) = default;

		void Initialize(Device*, const std::vector<std::vector<DescriptorRangeType>>&,const std::vector<StaticSamplerType>&);

		ID3D12RootSignature* Get();
	};


	//
	//
	//

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
			ID3D12RootSignature* tmp = nullptr;
			auto result = device->Get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&tmp));
			if (FAILED(result))
				throw "";
			root_signature_ptr.reset(tmp);

			rootSigBlob->Release();
		}
	}

	inline ID3D12RootSignature* RootSignature::Get()
	{
		return root_signature_ptr.get();
	}

}