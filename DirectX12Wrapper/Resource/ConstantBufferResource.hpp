#pragma once
#include"UploadResource.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	class ConstantBufferResource : public UploadResource
	{
	public:
		void Initialize(Device*, std::size_t size);
	};

	template<>
	struct ViewTypeTraits<ConstantBufferResource>
	{
		using Type = DescriptorHeapViewTag::ConstantBuffer;
	};

	//
	//
	//

	inline void ConstantBufferResource::Initialize(Device* device, std::size_t size)
	{
		//サイズは16の倍数じゃあないといけないのでアライメント
		size = (size + 0xff) & ~0xff;

		return UploadResource::Initialize(device, size);
	}
}