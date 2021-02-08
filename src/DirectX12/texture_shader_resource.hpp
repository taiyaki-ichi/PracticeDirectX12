#pragma once
#include"device.hpp"
#include"utility.hpp"
#include"command_list.hpp"
#include"resource_type_tag.hpp"
#include"descriptor_heap.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")


namespace ichi
{
	class device;
	class command_list;

	namespace create_view_type {
		struct SRV;
	}

	//�e�N�X�`���̃��\�[�X
	//�e�N�X�`����p�̂ŃX�N���v�^�q�[�v��View�����
	//�A�b�v���[�h�p�̒��ԃo�b�t�@�Ȃ�true
	template<bool isUpload>
	class texture_shader_resource_base
	{
		ID3D12Resource* m_resource = nullptr;

		//���\�[�X�o���A�̂Ƃ��g�p
		D3D12_RESOURCE_STATES m_resource_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

		//�e�N�X�`�����R�s�[����Ƃ��Ɏg�p
		D3D12_TEXTURE_COPY_LOCATION m_copy_location{};

	public:
		using resource_type = shader_resource_tag;
		using create_view_type = typename create_view_type::SRV;

		texture_shader_resource_base() = default;
		~texture_shader_resource_base();

		//device���ŌĂ΂��
		//�ǂ����������Ձ[�s����������A�����Ƃ��|�C���^�ł�������
		bool initialize(device*, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage);

		bool map(const DirectX::Image& image);

		//�f�B�X�N���v�^�q�[�v�Ɋ֘A�t����Ƃ��Ɏg�p
		ID3D12Resource* get() const noexcept;

		//���\�[�X�΁[�肠
		void resource_barrier(command_list*, D3D12_RESOURCE_STATES);

		D3D12_RESOURCE_STATES get_state();
		D3D12_TEXTURE_COPY_LOCATION& get_copy_location();
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

		//�摜�̃|�C���^
		auto image = scratchImage->GetImage(0, 0, 0);

		//���ԃo�b�t�@�p
		if constexpr (isUpload) {

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

			m_resource_state = D3D12_RESOURCE_STATE_GENERIC_READ;

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

			m_resource_state = D3D12_RESOURCE_STATE_COPY_DEST;
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

		//�R�s�[�p�̍\���̂̐ݒ�
		if constexpr (isUpload) {

			m_copy_location.pResource = m_resource;
			m_copy_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = m_resource->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			m_copy_location.PlacedFootprint = footprint;
			m_copy_location.PlacedFootprint.Offset = 0;
			m_copy_location.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
			m_copy_location.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
			m_copy_location.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
			m_copy_location.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			m_copy_location.PlacedFootprint.Footprint.Format = image->format;
		}
		else {
			
			m_copy_location.pResource = m_resource;
			m_copy_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			m_copy_location.SubresourceIndex = 0;
		}

		return true;
	}

	template<bool isUpload>
	inline bool texture_shader_resource_base<isUpload>::map(const DirectX::Image& image)
	{
		return map_func(m_resource, image);
	}


	template<bool isUpload>
	inline ID3D12Resource* texture_shader_resource_base<isUpload>::get() const noexcept
	{
		return m_resource;
	}

	template<bool isUpload>
	inline void texture_shader_resource_base<isUpload>::resource_barrier(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		if (m_resource_state == state)
			return;

		//���\�[�X�o���A�̍쐻
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = m_resource_state;
		BarrierDesc.Transition.StateAfter = state;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		m_resource_state = state;
	}

	template<bool isUpload>
	inline D3D12_RESOURCE_STATES texture_shader_resource_base<isUpload>::get_state()
	{
		return m_resource_state;
	}

	template<bool isUpload>
	inline D3D12_TEXTURE_COPY_LOCATION& texture_shader_resource_base<isUpload>::get_copy_location()
	{
		return m_copy_location;
	}


	//�e�N�X�`���p�̃C���^�[�t�F�[�X
	using texture_shader_resource = texture_shader_resource_base<false>;
	//�A�b�v���[�h����钆�ԃo�b�t�@�p�̃C���^�[�t�F�[�X
	using upload_texture_shader_resource = texture_shader_resource_base<true>;


}


