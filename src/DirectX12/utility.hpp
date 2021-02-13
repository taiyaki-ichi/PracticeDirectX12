#pragma once
#include<optional>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace DX12
{
	//�A�����W�����g
	inline constexpr size_t alignment_size(size_t size, size_t alignment) {
		return size + alignment - size % alignment;
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