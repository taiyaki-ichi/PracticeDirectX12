#pragma once
#include"UploadResource.hpp"

namespace DX12
{
	//1つ当たり32bitのインデックス、大きすぎるかも
	class IndexBuffer : public UploadResource
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	public:
		void Initialize(Device*, std::uint32_t indexNum, std::uint32_t stride = 4, Format format = { Type::Uint32,1 });

		const D3D12_INDEX_BUFFER_VIEW& GetView() const noexcept;
	};

	//
	//
	//

	inline void IndexBuffer::Initialize(Device* device, std::uint32_t indexNum, std::uint32_t stride , Format format)
	{
		UploadResource::Initialize(device, stride * indexNum);

		indexBufferView.BufferLocation = Get()->GetGPUVirtualAddress();
		indexBufferView.Format = format.value;
		indexBufferView.SizeInBytes = stride * indexNum;
	}

	inline const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::GetView() const noexcept
	{
		return indexBufferView;
	}
}