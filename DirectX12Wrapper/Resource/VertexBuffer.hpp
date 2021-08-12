#pragma once
#include"UploadResource.hpp"

namespace DX12
{
	class VertexBuffer : public UploadResource
	{
		D3D12_VERTEX_BUFFER_VIEW bufferView{};

	public:
		//stride‚Í‚P’¸“_‚ ‚½‚è‚Ì‘å‚«‚³
		void Initialize(Device*, std::uint32_t vertexNum, std::uint32_t stride);

		const D3D12_VERTEX_BUFFER_VIEW& GetView() const noexcept;
	};

	//
	//
	//

	inline void VertexBuffer::Initialize(Device* device, std::uint32_t vertexNum, std::uint32_t stride)
	{
		UploadResource::Initialize(device, vertexNum * stride);

		bufferView.BufferLocation = Get()->GetGPUVirtualAddress();
		bufferView.SizeInBytes = vertexNum * stride;
		bufferView.StrideInBytes = stride;
	}

	inline const D3D12_VERTEX_BUFFER_VIEW& VertexBuffer::GetView() const noexcept
	{
		return bufferView;
	}


}