#pragma once
#include"device.hpp"
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace ichi
{
	class device;

	//�e�N�X�`���̃��\�[�X
	//�e�N�X�`����p�̂ŃX�N���v�^�q�[�v��View�����
	//�A�b�v���[�h�p�̒��ԃo�b�t�@�Ȃ�true
	template<bool isUpload>
	class texture_shader_resource_base
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		texture_shader_resource_base() = default;
		~texture_shader_resource_base();

		//device���ŌĂ΂��
		bool initialize(device*, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage);

		//�f�B�X�N���v�^�q�[�v�Ɋ֘A�t����Ƃ��Ɏg�p
		ID3D12Resource* get() const noexcept;
	};



	template<bool isUpload>
	inline texture_shader_resource_base<isUpload>::~texture_shader_resource_base()
	{
		if (m_resource)
			m_resource->Release();
	}

	template<bool isUpload>
	inline bool texture_shader_resource_base<isUpload>::initialize(device* device, const DirectX::TexMetadata* metaData,const DirectX::ScratchImage* scratchImage)
	{
		D3D12_HEAP_PROPERTIES heapprop{};
		D3D12_RESOURCE_DESC resdesc{};
		D3D12_RESOURCE_STATES resourceState;

		//���ԃo�b�t�@�p
		if constexpr (isUpload) {
			//�摜�̃|�C���^
			auto image = scratchImage->GetImage(0, 0, 0);

			//�}�b�v�\�ɂ���
			heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
			//�A�b�v���[�h�p�Ȃ̂�UNKNOWN
			heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			//�P��A�_�v�^�[�̂��߂��ꂼ��0
			heapprop.CreationNodeMask = 0;
			heapprop.VisibleNodeMask = 0;

			//�P�Ȃ�f�[�^
			resdesc.Format = DXGI_FORMAT_UNKNOWN;
			//�P�Ȃ�o�b�t�@
			resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			//�f�[�^�T�C�Y
			resdesc.Width = static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height;
			resdesc.Height = 1;
			resdesc.DepthOrArraySize = 1;
			resdesc.MipLevels = 1;
			//�A�������f�[�^
			resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			//�A�b�v���[�h�p
			resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		}
		else {
			//�e�N�X�`���p
			heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
			//��������UNKNOWN
			heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			//�P��A�_�v�^�[�̂��߂��ꂼ��0
			heapprop.CreationNodeMask = 0;
			heapprop.VisibleNodeMask = 0;

			resdesc.Format = metaData->format;
			resdesc.Width = static_cast<UINT>(metaData->width);//��
			resdesc.Height = static_cast<UINT>(metaData->height);//����
			resdesc.DepthOrArraySize = static_cast<UINT16>(metaData->arraySize);//2D�Ŕz��ł��Ȃ��̂łP
			resdesc.MipLevels = static_cast<UINT16>(metaData->mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
			resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);//2D�e�N�X�`���p
			resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

			//�R�s�[��
			resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		}

		//���ɂȂ�
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		//���ʂ̃e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			resourceState,
			nullptr,
			IID_PPV_ARGS(&m_resource)))) {
			std::cout << "texture init is falied\n";
			return false;
		}

		return true;
	}

	template<bool isUpload>
	inline ID3D12Resource* texture_shader_resource_base<isUpload>::get() const noexcept
	{
		return m_resource;
	}

	//�e�N�X�`���p�̃C���^�[�t�F�[�X
	using texture_shader_resource = texture_shader_resource_base<false>;
	//�A�b�v���[�h����钆�ԃo�b�t�@�p�̃C���^�[�t�F�[�X
	using upload_texture_shader_resource = texture_shader_resource_base<true>;

}
