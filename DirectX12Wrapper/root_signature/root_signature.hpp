#pragma once
#include"device.hpp"
#include"root_signature_helper.hpp"
#include<string>

namespace DX12
{

	enum class descriptor_range_type {
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

	class root_signature
	{
		release_unique_ptr<ID3D12RootSignature> root_signature_ptr{};

	public:
		root_signature() = default;
		~root_signature() = default;

		root_signature(root_signature&&) = default;
		root_signature& operator=(root_signature&&) = default;

		void initialize(device&, const std::vector<std::vector<descriptor_range_type>>&,const std::vector<StaticSamplerType>&);

		ID3D12RootSignature* Get();
	};


	//
	//
	//

	void root_signature::initialize(device& device, const std::vector<std::vector<descriptor_range_type>>& descriptorRangeTypes, 
		const std::vector<StaticSamplerType>& staticSamplerTypes)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		auto descriptorRanges = get_descriptor_range(descriptorRangeTypes);
		auto descriptorTables = get_descriptor_tables(descriptorRanges);
		auto [descriptorTableData, descriptorTableSize] = get_container_data_and_size(descriptorTables);
		rootSignatureDesc.pParameters = descriptorTableData;
		rootSignatureDesc.NumParameters = descriptorTableSize;

		auto staticSamplers = get_static_samplers(staticSamplerTypes);
		auto [staticSamplerData, staticSamplerSize] = get_container_data_and_size(staticSamplers);
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

}