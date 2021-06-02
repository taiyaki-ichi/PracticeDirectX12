#pragma once
#include"UploadResource.hpp"

namespace DX12
{
	class VertexBufferResource : public UploadResource
	{
		D3D12_VERTEX_BUFFER_VIEW bufferView{};

	public:
		void Initialize(Device*, std::size_t size, std::size_t stride);

		const D3D12_VERTEX_BUFFER_VIEW& GetView() const noexcept;
	};

	//
	//
	//

	inline void VertexBufferResource::Initialize(Device* device, std::size_t size, std::size_t stride)
	{
		UploadResource::Initialize(device, size);

		bufferView.BufferLocation = Get()->GetGPUVirtualAddress();
		bufferView.SizeInBytes = size;
		bufferView.StrideInBytes = stride;
	}

	inline const D3D12_VERTEX_BUFFER_VIEW& VertexBufferResource::GetView() const noexcept
	{
		return bufferView;
	}


}