#pragma once
#include"ResourceBase.hpp"
#include"../Utility.hpp"

namespace DX12
{

	//インデックスバッファやバーテックスの基底クラスとなる
	//また、テクスチャのアップロード用
	class UploadResource : public ResourceBase
	{
	public:
		UploadResource() = default;
		virtual ~UploadResource() = default;

		UploadResource(UploadResource&&) noexcept = default;
		UploadResource& operator=(UploadResource&&) noexcept = default;

		void Initialize(Device*, std::size_t size);

		//コンテナ、配列用
		template<typename T>
		void Map(T&& container);

		//テクスチャデータ用
		void Map(uint8_t* data, std::size_t rowPitch, std::size_t height);
	};

	//
	//
	//

	void UploadResource::Initialize(Device* device, std::size_t size)
	{
		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resdesc.Width = size;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ResourceBase::Initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr
		);
	}

	template<typename T>
	inline void UploadResource::Map(T&& container)
	{
		using value_type = std::remove_reference_t<decltype(*std::begin(container))>;

		value_type* target = nullptr;
		auto result = Get()->Map(0, nullptr, (void**)&target);

		if (FAILED(result))
			throw "UploadResource::Map is failed\n";

		std::copy(std::begin(container), std::end(container), target);
		Get()->Unmap(0, nullptr);
	}

	void UploadResource::Map(uint8_t* data, std::size_t rowPitch, std::size_t height)
	{
		uint8_t* target = nullptr;
		if (FAILED(Get()->Map(0, nullptr, (void**)&target)))
			throw "UploadResource is failed\n";

		std::size_t targetRowPitch = AlignmentSize(rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (std::size_t i = 0; i < height; i++)
		{
			std::copy_n(data, rowPitch, target);
			data += rowPitch;
			target += targetRowPitch;
		}

		Get()->Unmap(0, nullptr);
	}


}