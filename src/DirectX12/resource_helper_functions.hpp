#pragma once
#include"resource.hpp"
#include<utility>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include<iostream>


namespace ichi
{
	class resource;
	class device;
	class command_list;

	
	//���\�[�X��萔�o�b�t�@�Ƃ��č쐻
	resource* create_constant_resource(device*, unsigned int size);

	//�e�N�X�`���̃V�F�[�_���\�[�X�̎擾
	resource* create_texture_resource(device*, command_list*, const DirectX::TexMetadata*, const DirectX::ScratchImage*);

	//�t�H�[�}�b�g�Ȃǂ��w�肷�邱�Ƃ�rtv�p�̃��\�[�X��؂�|���S���̃��\�[�X�ȂǂɎg�p�ł���
	//�����Ƃ��Ƃ肠�������Ċ���
	resource* create_simple_resource(device*, unsigned int width, unsigned int height, 
		DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flag,D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE clearValue);

	//�[�x�o�b�t�@�̐���
	resource* crate_depth_resource(device*, unsigned int width, unsigned int height);
	

	//���\�[�X�ւ̃}�b�v
	//�s��p
	bool map_to_resource(resource*, const DirectX::XMMATRIX&);

	//�e�N�X�`���p
	bool map_to_resource(resource*, const DirectX::Image&);


	//T�̓R���e�i�A�܂��͔z��p
	template<typename T>
	bool map_to_resource(resource* resource, T&& container)
	{
		using value_type = std::remove_reference_t<decltype(*std::begin(container))>;

		value_type* target = nullptr;
		auto result = resource->get()->Map(0, nullptr, (void**)&target);
		//���s�����Ƃ�
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(container), std::end(container), target);
		resource->get()->Unmap(0, nullptr);

		return true;
	}






}