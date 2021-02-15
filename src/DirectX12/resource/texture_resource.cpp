#include"texture_resource.hpp"
#include"upload_resource.hpp"
#include"../utility.hpp"
#include"../device.hpp"
#include"../command_list.hpp"

namespace DX12
{
	bool texture_resource::initialize(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage)
	{
		//画像のポインタ
		auto image = scratchImage->GetImage(0, 0, 0);

		//中間のバッファ
		upload_resource src{};
		src.initialize(device, static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height);

		//データをマップ
		src.map(*image);

	
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;
		resdesc.Format = metaData->format;
		resdesc.Width = static_cast<UINT>(metaData->width);//幅
		resdesc.Height = static_cast<UINT>(metaData->height);//高さ
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData->arraySize);//2Dで配列でもないので１
		resdesc.MipLevels = static_cast<UINT16>(metaData->mipLevels);//ミップマップしないのでミップ数は１つ
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);//2Dテクスチャ用
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		//こっちもUNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//単一アダプターのためそれぞれ0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		ID3D12Resource* dstPtr = nullptr;

		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&dstPtr)))) {
			return false;
		}

		//
		//コピーのための構造体の設定
		//

		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};

		srcLocation.pResource = src.get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = dstPtr->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
		srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
		srcLocation.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		srcLocation.PlacedFootprint.Footprint.Format = image->format;

		dstLocation.pResource = dstPtr;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		//
		//戻り値のリソースにコピーするコマンドを実行
		//
		cl->get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
		cl->get()->Close();
		cl->execute();
		cl->clear();


		return resource_base::initialize(dstPtr);
	}
}