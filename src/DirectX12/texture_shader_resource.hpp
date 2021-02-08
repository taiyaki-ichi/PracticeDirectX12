#pragma once
#include"device.hpp"
#include"utility.hpp"
#include"command_list.hpp"
#include"resource_type_tag.hpp"
#include"descriptor_heap.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")


namespace ichi
{
	class device;
	class command_list;

	namespace create_view_type {
		struct SRV;
	}

	//テクスチャのリソース
	//テクスチャ専用のでスクリプタヒープにViewを作る
	//アップロード用の中間バッファならtrue
	template<bool isUpload>
	class texture_shader_resource_base
	{
		ID3D12Resource* m_resource = nullptr;

		//リソースバリアのとき使用
		D3D12_RESOURCE_STATES m_resource_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

		//テクスチャをコピーするときに使用
		D3D12_TEXTURE_COPY_LOCATION m_copy_location{};

	public:
		using resource_type = shader_resource_tag;
		using create_view_type = typename create_view_type::SRV;

		texture_shader_resource_base() = default;
		~texture_shader_resource_base();

		//device内で呼ばれる
		//どっちかがこぷー不可だったから、両方ともポインタでを引数に
		bool initialize(device*, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage);

		bool map(const DirectX::Image& image);

		//ディスクリプタヒープに関連付けるときに使用
		ID3D12Resource* get() const noexcept;

		//リソースばーりあ
		void resource_barrier(command_list*, D3D12_RESOURCE_STATES);

		D3D12_RESOURCE_STATES get_state();
		D3D12_TEXTURE_COPY_LOCATION& get_copy_location();
	};




	template<bool isUpload>
	inline texture_shader_resource_base<isUpload>::~texture_shader_resource_base()
	{
		if (m_resource)
			m_resource->Release();
	}

	template<bool isUpload>
	inline bool texture_shader_resource_base<isUpload>::initialize(device* device, const DirectX::TexMetadata* metaData,const DirectX::ScratchImage* scratchImage)
	{
		D3D12_HEAP_PROPERTIES heapprop{};
		D3D12_RESOURCE_DESC resdesc{};
		D3D12_RESOURCE_STATES resourceState;

		//画像のポインタ
		auto image = scratchImage->GetImage(0, 0, 0);

		//中間バッファ用
		if constexpr (isUpload) {

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
			
			//アップロード用
			resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

			m_resource_state = D3D12_RESOURCE_STATE_GENERIC_READ;

		}
		else {
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

			//コピー先
			resourceState = D3D12_RESOURCE_STATE_COPY_DEST;

			m_resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
		}

		//特になし
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		//普通のテクスチャなのでアンチエイリアシングしない
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			resourceState,
			nullptr,
			IID_PPV_ARGS(&m_resource)))) {
			std::cout << "texture init is falied\n";
			return false;
		}

		//コピー用の構造体の設定
		if constexpr (isUpload) {

			m_copy_location.pResource = m_resource;
			m_copy_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = m_resource->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			m_copy_location.PlacedFootprint = footprint;
			m_copy_location.PlacedFootprint.Offset = 0;
			m_copy_location.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
			m_copy_location.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
			m_copy_location.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
			m_copy_location.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			m_copy_location.PlacedFootprint.Footprint.Format = image->format;
		}
		else {
			
			m_copy_location.pResource = m_resource;
			m_copy_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			m_copy_location.SubresourceIndex = 0;
		}

		return true;
	}

	template<bool isUpload>
	inline bool texture_shader_resource_base<isUpload>::map(const DirectX::Image& image)
	{
		return map_func(m_resource, image);
	}


	template<bool isUpload>
	inline ID3D12Resource* texture_shader_resource_base<isUpload>::get() const noexcept
	{
		return m_resource;
	}

	template<bool isUpload>
	inline void texture_shader_resource_base<isUpload>::resource_barrier(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		if (m_resource_state == state)
			return;

		//リソースバリアの作製
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = m_resource_state;
		BarrierDesc.Transition.StateAfter = state;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		m_resource_state = state;
	}

	template<bool isUpload>
	inline D3D12_RESOURCE_STATES texture_shader_resource_base<isUpload>::get_state()
	{
		return m_resource_state;
	}

	template<bool isUpload>
	inline D3D12_TEXTURE_COPY_LOCATION& texture_shader_resource_base<isUpload>::get_copy_location()
	{
		return m_copy_location;
	}


	//テクスチャ用のインターフェース
	using texture_shader_resource = texture_shader_resource_base<false>;
	//アップロードされる中間バッファ用のインターフェース
	using upload_texture_shader_resource = texture_shader_resource_base<true>;


}


