#include"resource_helper_functions.hpp"
#include"utility.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace ichi
{

	resource* create_constant_resource(device* device, unsigned int size)
	{
		auto result = new resource{};

		//サイズは16の倍数じゃあないといけないのでアライメント
		size = (size + 0xff) & ~0xff;

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

		if (!result->initialize(device,
			&heapprop, 
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			nullptr))
		{
			std::cout << __func__ << " is failed\n";
		}

		return result;
	}

	/*
	resource* create_texture_resource(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage)
	{
		//中間バッファ用
		resource src{};
		//実際のデータ
		auto dst = new resource{};

		//画像のポインタ
		auto image = scratchImage->GetImage(0, 0, 0);

		D3D12_HEAP_PROPERTIES heapprop{};
		D3D12_RESOURCE_DESC resdesc{};
		//プロパティーの共通部分
		//特になし
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		//普通のテクスチャなのでアンチエイリアシングしない
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;


		//
		//まずは中間バッファを作製
		//
		//マップ可能にする
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		//アップロード用なのでUNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//単一アダプターのためそれぞれ0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;
		//単なるデータ
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		//単なるバッファ
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		//データサイズ
		resdesc.Width = static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		//連続したデータ
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//アップロードするのでリソースの状態はREAD
		src.initialize(device, &heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);


		//
		//次に戻り値となるリソースの初期化
		//
		//テクスチャ用
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		//こっちもUNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//単一アダプターのためそれぞれ0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;
		resdesc.Format = metaData->format;
		resdesc.Width = static_cast<UINT>(metaData->width);//幅
		resdesc.Height = static_cast<UINT>(metaData->height);//高さ
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData->arraySize);//2Dで配列でもないので１
		resdesc.MipLevels = static_cast<UINT16>(metaData->mipLevels);//ミップマップしないのでミップ数は１つ
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);//2Dテクスチャ用
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		//初期状態はDEST
		dst->initialize(device, &heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);


		//
		//テクスチャのデータをマップ
		//
		map_to_resource(&src, image);


		//
		//テクスチャをコピーするためそれぞれのロケーションの構造体の設定
		//
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};
		
		srcLocation.pResource = src.get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = dst->get()->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
		srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
		srcLocation.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		srcLocation.PlacedFootprint.Footprint.Format = image->format;

		dstLocation.pResource = dst->get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;


		//
		//戻り値のリソースにコピーするコマンドを実行
		//

		cl->clear();

		//dst->resource_barrier(this, D3D12_RESOURCE_STATE_COPY_DEST);
		cl->get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
		//dst->resource_barrier(this, D3D12_RESOURCE_STATE_GENERIC_READ);

		cl->get()->Close();
		cl->execute();
		cl->clear();

		return dst;
	}
	*/

}