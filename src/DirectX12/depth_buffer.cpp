#include"depth_buffer.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace ichi
{
	depth_buffer::~depth_buffer()
	{
		if (m_resource)
			m_resource->Release();
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
	}
	bool depth_buffer::initialize(device* device,unsigned int windowWidth,unsigned int windowHeight)
	{
		//�[�x�o�b�t�@�̎d�l
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		depthResDesc.Width = windowWidth;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
		depthResDesc.Height = windowHeight;//��ɓ���
		depthResDesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;//�[�x�l�������ݗp�t�H�[�}�b�g
		depthResDesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//���̃o�b�t�@�͐[�x�X�e���V���Ƃ��Ďg�p���܂�
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		//�f�v�X�p�q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//���̃N���A�o�����[���d�v�ȈӖ�������
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;//�[���P(�ő�l)�ŃN���A
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue,
			IID_PPV_ARGS(&m_resource)
		))) {
			std::cout << "depth_buffer initialize is failed\n";
			return false;
		}

		//�[�x�̂��߂̃f�X�N���v�^�q�[�v�쐬
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};//�[�x�Ɏg����Ƃ��������킩��΂���
		dsvHeapDesc.NumDescriptors = 1;//�[�x�r���[1�̂�
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�f�v�X�X�e���V���r���[�Ƃ��Ďg��
		if (FAILED(device->get()->CreateDescriptorHeap(
			&dsvHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << "depth_buffer descriptor is failed\n";
			return false;
		}
		
		//�[�x�r���[�쐬
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�

		device->get()->CreateDepthStencilView(
			m_resource, &dsvDesc, m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

		return true;
	}

	void depth_buffer::clear(command_list* cl)
	{
		cl->get()->ClearDepthStencilView(
			m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE depth_buffer::get_cpu_descriptor_handle()
	{
		return m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	}
	ID3D12Resource* depth_buffer::get()
	{
		return m_resource;
	}
}