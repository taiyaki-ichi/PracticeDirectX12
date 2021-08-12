#pragma once
#include"UploadResource.hpp"

namespace DX12
{
	//1つ当たり32bitのインデックス、大きすぎるかも
	class IndexBuffer : public UploadResource
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	public:
		void Initialize(Device*, std::uint32_t indexNum);

		const D3D12_INDEX_BUFFER_VIEW& GetView() const noexcept;
	};

	//
	//
	//

	inline void IndexBuffer::Initialize(Device* device, std::uint32_t indexNum)
	{
		UploadResource::Initialize(device, 4 * indexNum);

		indexBufferView.BufferLocation = Get()->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = 4 * indexNum;
	}

	inline const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetView() const noexcept
	{
		return indexBufferView;
	}
}