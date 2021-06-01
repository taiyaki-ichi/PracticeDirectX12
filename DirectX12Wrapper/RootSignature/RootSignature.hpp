#pragma once
#include"Device.hpp"
#include"RootSignatureInitializeHelper.hpp"
#include<string>

namespace DX12
{
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

		template<typename DescriptorTableType, typename StaticSamplersType>
		void Initialize(Device*);

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


	template<typename DescriptorTableType, typename StaticSamplersType>
	inline void RootSignature::Initialize(Device* device)
	{
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		constexpr auto rootParameters = GetDescriptorTableTuple<DescriptorTableType>();
		auto descriptorTables = GetDescriptorTableStructArray(rootParameters);
		rootSignatureDesc.pParameters = descriptorTables.data();
		rootSignatureDesc.NumParameters = descriptorTables.size();

		constexpr auto staticSamplers = GetStaticSamplerArray<StaticSamplersType>();
		rootSignatureDesc.pStaticSamplers = staticSamplers.data();
		rootSignatureDesc.NumStaticSamplers = staticSamplers.size();

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
				throw "CreateRootSignature is falied : CreateRootSignature\n";

			rootSigBlob->Release();
		}
	}

	inline ID3D12RootSignature* RootSignature::Get()
	{
		return rootSignature;
	}

}