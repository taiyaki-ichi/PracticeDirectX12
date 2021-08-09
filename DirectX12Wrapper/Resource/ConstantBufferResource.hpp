#pragma once
#include"UploadResource.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	class ConstantBufferResource : public UploadResource
	{
	public:
		void Initialize(Device*, std::uint32_t size);
	};

	template<>
	struct DefaultViewTypeTraits<ConstantBufferResource>
	{
		using Type = DescriptorHeapViewTag::ConstantBuffer;
	};

	//
	//
	//

	inline void ConstantBufferResource::Initialize(Device* device, std::uint32_t size)
	{
		//�T�C�Y��16�̔{�����Ⴀ�Ȃ��Ƃ����Ȃ��̂ŃA���C�����g
		size = (size + 0xff) & ~0xff;

		return UploadResource::Initialize(device, size);
	}
}