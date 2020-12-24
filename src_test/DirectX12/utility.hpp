#pragma once
#include<type_traits>
#include<optional>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace ichi
{
	//�A�����W�����g
	inline constexpr size_t alignment_size(size_t size, size_t alignment) {
		return size + alignment - size % alignment;
	}

	//�o�b�t�@�ɏ����}�b�v
	//T�̓R���e�i
	template<typename T>
	inline  bool map_func(ID3D12Resource* buffer, T&& t)
	{
		using value_type = std::remove_reference_t<decltype(t[0])>;

		value_type* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//���s�����Ƃ�
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(t), std::end(t), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

	//�s��p
	template<>
	inline bool map_func<DirectX::XMMATRIX&>(ID3D12Resource* buffer, DirectX::XMMATRIX& m)
	{
		DirectX::XMMATRIX* ptr = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&ptr))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		*ptr = m;
		buffer->Unmap(0, nullptr);

		return true;
	}

	//�e�N�X�`���p
	inline bool map_func(ID3D12Resource* buffer, const DirectX::Image& image) {

		uint8_t* mapforImage = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		uint8_t* srcAddress = image.pixels;
		size_t rowPitch = alignment_size(image.rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (int y = 0; y < image.height; ++y) {
			std::copy_n(srcAddress,
				rowPitch,
				mapforImage);//�R�s�[
			//1�s���Ƃ̒�������킹�Ă��
			srcAddress += image.rowPitch;
			mapforImage += rowPitch;
		}

		buffer->Unmap(0, nullptr);

		return true;
	}

	

	//�摜�f�[�^�̎擾
	//���ƁAScratchImage�̓R�s�[�s��
	inline std::optional<std::pair<DirectX::TexMetadata, DirectX::ScratchImage>> 
		get_texture(const wchar_t* fileName) {

		DirectX::TexMetadata metaData{};
		DirectX::ScratchImage scratch{};
		if (FAILED(LoadFromWICFile(fileName, DirectX::WIC_FLAGS_NONE, &metaData, scratch))) {
			return std::nullopt;
		}
		return std::make_pair(std::move(metaData), std::move(scratch));
	}


}