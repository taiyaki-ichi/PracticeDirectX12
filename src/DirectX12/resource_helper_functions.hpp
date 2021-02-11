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

	//�C���f�b�N�X�o�b�t�@�Ƃ��̃r���[�̍쐻
	//std::pair<resource*, D3D12_INDEX_BUFFER_VIEW> create_index_resource_and_view(device*, unsigned int size);

	//���_�o�b�t�@�Ƃ��̃r���[�̍쐻
	//std::pair<resource*, D3D12_VERTEX_BUFFER_VIEW> create_vertex_resource_and_view(device*, unsigned int size, unsigned int stride);

	//�����_�[�^�[�Q�b�g�p�̃��\�[�X�̍쐻
	//resource* create_rtv_resource(device*, D3D12_CLEAR_VALUE);

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