#pragma once
#include"UploadResource.hpp"

namespace DX12
{
	class IndexBuffer : public UploadResource
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	public:
		void Initialize(Device*, std::uint32_t size);

		const D3D12_INDEX_BUFFER_VIEW& GetView() const noexcept;
	};

	//
	//
	//

	inline void IndexBuffer::Initialize(Device* device, std::uint32_t size)
	{
		UploadResource::Initialize(device, size);

		indexBufferView.BufferLocation = Get()->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		indexBufferView.SizeInBytes = size;
	}

	inline const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetView() const noexcept
	{
		return indexBufferView;
	}
}