#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>
#include"descriptor_heap.hpp"
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace graphics::test001
{
	
	//�f�B�X�N���v�^�̍\���̒ʒm�p�̍\���̎擾
	//�e�N�X�`���A�s��萔�A�[�x�X�e���V���o�b�t�@�̏�
	//���[�g�V�O�l�`���ɂԂ�����
	std::array<D3D12_DESCRIPTOR_RANGE, 3> get_descriptor_ranges()
	{
		//�e�N�X�`��
		D3D12_DESCRIPTOR_RANGE textureDesc{};
		textureDesc.NumDescriptors = 1;
		textureDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//SRV��0�Ԗ�
		textureDesc.BaseShaderRegister = 0;
		textureDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//���W�̍s��
		D3D12_DESCRIPTOR_RANGE matDesc{};
		matDesc.NumDescriptors = 1;
		matDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		//CBV(�萔�o�b�t�@)��0�Ԗ�
		matDesc.BaseShaderRegister = 0;
		matDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//�[�x�X�e���V���o�b�t�@
		D3D12_DESCRIPTOR_RANGE stencilDesc{};
		stencilDesc.NumDescriptors = 1;
		stencilDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//SRV��0�Ԗ�
		stencilDesc.BaseShaderRegister = 0;
		stencilDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

}