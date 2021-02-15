#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;
}

using namespace DX12;

namespace DX12
{
	


	//���\�[�X�̃x�[�X�̃N���X
	class resource_base
	{
		//�{��
		ID3D12Resource* m_resource = nullptr;

		//���\�[�X�o���A�̂Ƃ��g�p
		D3D12_RESOURCE_STATES m_state{};

		//����Ȃ�������폜�����
		std::optional<D3D12_CLEAR_VALUE> m_clear_value{};

	public:
		resource_base() = default;
		virtual ~resource_base();
		//�R�s�[�֎~
		resource_base(const resource_base&) = delete;
		resource_base& operator=(const resource_base&) = delete;
		//���[�u�\
		resource_base(resource_base&&) noexcept;
		resource_base& operator=(resource_base&&) noexcept;

		//������
		bool initialize(device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag,
			const D3D12_RESOURCE_DESC* resoDesc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue);

		//���̂Ƃ���o�b�N�o�b�t�@�̎��ƃe�N�X�`���̎�
		bool initialize(ID3D12Resource* resource);

		//���\�[�X�o���A
		void barrior(command_list*, D3D12_RESOURCE_STATES);

		ID3D12Resource* get() noexcept;

		D3D12_CLEAR_VALUE* get_clear_value();

		//�L���ȃ|�C���^��ێ����Ă��邩�ǂ���
		bool is_empty() const noexcept;
	};


}