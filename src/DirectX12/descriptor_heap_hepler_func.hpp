#pragma once
#include"device.hpp"
#include"descriptor_heap_type.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{


	//�f�B�X�N���v�^�q�[�v�Ƀr���[�����֐�
	//���ꉻ�ł��Ȃ��ꍇ���s
	template<typename DescriptorHeapType, typename CreateType>
	inline bool create_view_func(device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	//�f�B�X�N���v�^�q�[�v�̃^�C�v��CBV_SRV_UAV�̎���CBV�̃r���[�����
	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::constant_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//�萔�o�b�t�@
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);

		//�萔�o�b�t�@�r���[�̍쐬
		device->get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

		return true;
	}


	//�f�B�X�N���v�^�q�[�v�̃^�C�v��CBV_SRV_UAV�̎���SRV�̃r���[�����
	template<>
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::float4_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//�e�N�X�`��
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resource->GetDesc().Format;//RGBA(0.0f�`1.0f�ɐ��K��)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

		device->get()->CreateShaderResourceView(resource, //�r���[�Ɗ֘A�t����o�b�t�@
			&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			cpuHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		return true;
	}


	//�f�B�X�N���v�^�q�[�v�̃^�C�v��CBV_SRV_UAV�̎��ɐ[�x�o�b�t�@���V�F�[�_���\�[�X�Ƃ���
	//�������߂�View�̍쐻
	template<>
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::depth_stencil_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
		resDesc.Format = DXGI_FORMAT_R32_FLOAT;
		resDesc.Texture2D.MipLevels = 1;
		resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->get()->CreateShaderResourceView(resource, //�r���[�Ɗ֘A�t����o�b�t�@
			&resDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			cpuHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		return true;
	}

	//�[�x�o�b�t�@��[�x�o�b�t�@�Ƃ��Ďg������
	//�[�x�o�b�t�@�p�̃f�B�X�N���v�^�q�[�v��View�����p
	template<>
	inline bool  create_view_func<descriptor_heap_type::DSV, view_type::depth_stencil_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�

		device->get()->CreateDepthStencilView(resource, //�r���[�Ɗ֘A�t����o�b�t�@
			&dsvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			cpuHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		return true;
	}


	//�����_�[�^�[�Q�b�g�p�̃f�B�X�N���v�^�q�[�v��
	//�����_�[�^�[�Q�b�g��View�����
	template<>
	inline bool create_view_func<descriptor_heap_type::RTV, view_type::float4_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//SRGB�����_�[�^�[�Q�b�g�r���[�ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}


	template<>
	inline bool create_view_func<descriptor_heap_type::RTV, view_type::float_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//SRGB�����_�[�^�[�Q�b�g�r���[�ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::float_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

		device->get()->CreateShaderResourceView(resource, //�r���[�Ɗ֘A�t����o�b�t�@
			&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			cpuHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		return true;
	}




}