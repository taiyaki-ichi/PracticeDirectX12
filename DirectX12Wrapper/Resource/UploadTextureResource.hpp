#pragma once
#include"UploadResource.hpp"
#include"Utility.hpp"

namespace DX12
{
	//Texture���A�b�v���[�h����p
	class UploadTextureResource : public UploadResource
	{
	public:
		void Initialize(Device*, std::uint32_t width, std::uint32_t height);
	};

	//
	//
	//

	void UploadTextureResource::Initialize(Device* device, std::uint32_t width, std::uint32_t height)
	{
		UploadResource::Initialize(device, AlignmentSize<std::uint32_t>(width, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * height);
	}

}