#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;

	//���\�[�X�i�o�b�t�@�j�̃N���X
	class resource
	{
		//�{��
		ID3D12Resource* m_resource = nullptr;

		//���\�[�X�o���A�̂Ƃ��g�p
		D3D12_RESOURCE_STATES m_state{};

	public:
		resource() = default;
		virtual ~resource();

		//�R�s�[�֎~
		resource(const resource&) = delete;
		resource& operator=(const resource&) = delete;
		//���[�u�\
		resource(resource&&) noexcept;
		resource& operator=(resource&&) noexcept;

		//������
		bool initialize(device*, const D3D12_HEAP_PROPERTIES*, D3D12_HEAP_FLAGS,
			const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*);

		//���\�[�X����̏�����
		bool initialize(ID3D12Resource*);

		//���\�[�X�o���A
		void barrior(command_list*, D3D12_RESOURCE_STATES);

		ID3D12Resource* get() noexcept;

		//�L���ȃ|�C���^��ێ����Ă��邩�ǂ���
		bool is_empty() const noexcept;
	};


}