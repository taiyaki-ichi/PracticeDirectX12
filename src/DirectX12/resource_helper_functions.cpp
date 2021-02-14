#include"resource_helper_functions.hpp"
#include"utility.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace DX12
{

	resource create_constant_resource(device* device, unsigned int size)
	{
		resource result{};

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

		if (!result.initialize(device,
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


	resource create_texture_resource(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage)
	{
		//中間バッファ用
		resource src{};
		//実際のデータ
		resource dst{};

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
		dst.initialize(device, &heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);


		//
		//テクスチャのデータをマップ
		//
		map_to_resource(&src, *image);


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
			auto desc = dst.get()->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
		srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
		srcLocation.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		srcLocation.PlacedFootprint.Footprint.Format = image->format;

		dstLocation.pResource = dst.get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;


		//
		//戻り値のリソースにコピーするコマンドを実行
		//
		cl->get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
		cl->get()->Close();
		cl->execute();
		cl->clear();

		return dst;
	}
	

	resource create_simple_resource(device* device, unsigned int width, unsigned int height,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE clearValue)
	{
		resource result{};

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
		resdesc.Width = width;//幅と高さはレンダーターゲットと同じ
		resdesc.Height = height;//上に同じ
		resdesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
		resdesc.Format = format;
		resdesc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
		resdesc.Flags = flag;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


		if (!result.initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			state,
			&clearValue))
		{
			std::cout << __func__ << " is failed\n";
		}

		return result;
	}

	resource create_depth_resource(device* device, unsigned int width, unsigned int height)
	{
		resource result{};

		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
		depthResDesc.Width = width;
		depthResDesc.Height = height;
		depthResDesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;//深度値書き込み用フォーマット
		depthResDesc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//このバッファは深度ステンシルとして使用します
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		//デプス用ヒーププロパティ
		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//このクリアバリューが重要な意味を持つ
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.f;//深さ１(最大値)でクリア
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア
		depthClearValue.Color[0] = 1.f;
		depthClearValue.Color[1] = 1.f;
		depthClearValue.Color[2] = 1.f;
		depthClearValue.Color[3] = 1.f;

		if (!result.initialize(
			device,
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
			&depthClearValue
		)) {
			std::cout << "create depth initialize is failed\n";
		}

		return result;
	}


	bool map_to_resource(resource* resource, const DirectX::Image& image)
	{
		uint8_t* mapforImage = nullptr;
		if (FAILED(resource->get()->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		uint8_t* srcAddress = image.pixels;
		size_t rowPitch = alignment_size(image.rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (size_t y = 0; y < image.height; ++y) {
			std::copy_n(srcAddress,
				rowPitch,
				mapforImage);//コピー
			//1行ごとの辻褄を合わせてやる
			srcAddress += image.rowPitch;
			mapforImage += rowPitch;
		}

		resource->get()->Unmap(0, nullptr);

		return true;
	}

}