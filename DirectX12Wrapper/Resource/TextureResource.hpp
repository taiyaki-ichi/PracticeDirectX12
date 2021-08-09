#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"
#include"UploadResource.hpp"
#include"../Utility.hpp"

namespace DX12
{
	class TextureResource : public ResourceBase
	{
	public:
		void Initialize(Device* device, CommandList* cl, 
			std::uint8_t* imagePtr, std::uint32_t width, std::uint32_t height, std::uint32_t rowPitch);
	};
	
	template<>
	struct DefaultViewTypeTraits<TextureResource>
	{
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

	//
	//
	//

	inline void TextureResource::Initialize(Device* device, CommandList* cl,
		std::uint8_t* imagePtr, std::uint32_t width, std::uint32_t height, std::uint32_t rowPitch)
	{
		//いったんアップロードする用のバッファ
		UploadResource uploadResource{};
		//アップロードする用のバッファのrowPitch
		auto uploadResourceRowPitch = AlignmentSize(rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		uploadResource.Initialize(device, uploadResourceRowPitch * height);

		//アップロードする
		{
			std::uint8_t* uploadResourceImagePtr = nullptr;
			
			if (FAILED(uploadResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&uploadResourceImagePtr))))
				throw "";

			for (std::uint32_t i = 0; i < height; i++) {
				std::copy_n(imagePtr, rowPitch, uploadResourceImagePtr);
				imagePtr += rowPitch;
				uploadResourceImagePtr += uploadResourceRowPitch;
			}

			uploadResource.Get()->Unmap(0, nullptr);
		}

		//このクラスのリソースの初期化
		{
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Width = width;
			resourceDesc.Height = height;
			//
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;//D3D12_RESOURCE_FLAG_NONE;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			//変更できるようにした方がいいかも
			resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

			D3D12_HEAP_PROPERTIES heapProperties{};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			ResourceBase::Initialize(
				device,
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				ResourceState::CopyDest,
				nullptr
			);
		}

		//コピーを行う
		{

			D3D12_TEXTURE_COPY_LOCATION srcLocation{};
			D3D12_TEXTURE_COPY_LOCATION dstLocation{};

			srcLocation.pResource = uploadResource.Get();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			{
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
				UINT nrow;
				UINT64 rowsize, size;
				auto desc = Get()->GetDesc();
				device->Get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
				srcLocation.PlacedFootprint = footprint;
			}
			srcLocation.PlacedFootprint.Offset = 0;
			srcLocation.PlacedFootprint.Footprint.Width = width;
			srcLocation.PlacedFootprint.Footprint.Height = height;
			srcLocation.PlacedFootprint.Footprint.Depth = 1;
			srcLocation.PlacedFootprint.Footprint.RowPitch = uploadResourceRowPitch;
			srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			dstLocation.pResource = Get();
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLocation.SubresourceIndex = 0;

			//コピーの実行
			cl->Get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
			cl->Get()->Close();
			cl->Execute();
			cl->Clear();
		}

	}


}