#pragma once
#include"UploadResource.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	class ConstantBuffer : public UploadResource
	{
	public:
		void Initialize(Device*, std::uint32_t size);
	};

	template<>
	struct DefaultViewTypeTraits<ConstantBuffer>
	{
		using Type = DescriptorHeapViewTag::ConstantBuffer;
	};

	//
	//
	//

	inline void ConstantBuffer::Initialize(Device* device, std::uint32_t size)
	{
		//サイズは16の倍数じゃあないといけないのでアライメント
		size = (size + 0xff) & ~0xff;

		return UploadResource::Initialize(device, size);
	}
}