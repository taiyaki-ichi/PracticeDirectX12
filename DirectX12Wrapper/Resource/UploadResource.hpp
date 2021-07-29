#pragma once
#include"ResourceBase.hpp"
#include"../Utility.hpp"

#include<iostream>
#include<typeinfo>

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
		void Map(T&&);

		//テクスチャデータ用
		void Map(uint8_t* data, std::size_t rowPitch, std::size_t height);
	};

	//ヘルパ
	template<typename,typename =std::void_t<>>
	struct HasIterator :std::false_type {};

	template<typename T>
	struct HasIterator<T, std::void_t<typename std::remove_reference_t<T>::iterator>> :std::true_type {};

	template<typename T>
	void MapStruct(ID3D12Resource*, T&&);

	template<typename T>
	void MapContainer(ID3D12Resource*, T&&);

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
			ResourceState::GenericRead,
			nullptr
		);
	}

	template<typename T>
	inline void UploadResource::Map(T&& t)
	{
		if constexpr (HasIterator<T>::value)
			MapContainer(Get(), std::forward<T>(t));
		else
			MapStruct(Get(), std::forward<T>(t));
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

	template<typename T>
	void MapStruct(ID3D12Resource* resourcePtr,T&& t)
	{
		using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
		ValueType* target = nullptr;
		resourcePtr->Map(0, nullptr, (void**)&target);
		*target = std::forward<T>(t);
		resourcePtr->Unmap(0, nullptr);
	}

	template<typename T>
	void MapContainer(ID3D12Resource* resourcePtr, T&& t)
	{
		using ValueType = std::remove_reference_t<decltype(*std::begin(t))>;
		ValueType* target = nullptr;
		resourcePtr->Map(0, nullptr, (void**)&target);
		std::copy(std::begin(t), std::end(t), target);
		resourcePtr->Unmap(0, nullptr);
	}


}