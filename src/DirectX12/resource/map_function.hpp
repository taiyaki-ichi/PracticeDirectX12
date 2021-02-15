#pragma once
#include"../utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include<iostream>

namespace DX12
{

	//���\�[�X�ւ̃}�b�v

	//�e�N�X�`���p
	inline bool map_to_resource(ID3D12Resource* resource, const DirectX::Image& image)
	{
		uint8_t* mapforImage = nullptr;
		if (FAILED(resource->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		uint8_t* srcAddress = image.pixels;
		size_t rowPitch = alignment_size(image.rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (size_t y = 0; y < image.height; ++y) {
			std::copy_n(srcAddress,
				rowPitch,
				mapforImage);//�R�s�[
			//1�s���Ƃ̒�������킹�Ă��
			srcAddress += image.rowPitch;
			mapforImage += rowPitch;
		}

		resource->Unmap(0, nullptr);

		return true;
	}


	//T�̓R���e�i�A�܂��͔z��p
	template<typename T>
	bool map_to_resource(ID3D12Resource* resource, T&& container)
	{
		using value_type = std::remove_reference_t<decltype(*std::begin(container))>;

		value_type* target = nullptr;
		auto result = resource->Map(0, nullptr, (void**)&target);
		//���s�����Ƃ�
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(container), std::end(container), target);
		resource->Unmap(0, nullptr);

		return true;
	}
}