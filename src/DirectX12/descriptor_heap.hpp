#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
#include"resource_type_tag.hpp"
#include<optional>
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;


	//�^�C�v�w��p
	//�������p�̊֐��̒�`
	//�C���N�������g�̕��̎擾�p�֐�
	namespace descriptor_heap_type {
		struct CBV_SRV_UAV;
		struct DSV;
	};


	template<typename,typename>
	bool create_view_func(device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	//�^�C�v�̓e���v���[�g�Ŏw��
	template<class DescriptorHeapType>
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap() {
			if (m_descriptor_heap)
				m_descriptor_heap->Release();
		}

		bool initialize(device * d,unsigned int size);

		//CreateType�ɂ�create(device,handle)�̐ÓI�Ȋ֐�
		template<typename CreateType>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
			create_view(device*, ID3D12Resource*);

		//�^�C�v�������Ă���ꍇ�̃C���^�[�t�F�[�X�֐�
		//get�Ŏ��ۂ̃��\�[�X���擾�ł���悤��
		template<typename T, typename CreateType = typename T::create_view_type>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, T* resourcePtr);
		//�����Ă��Ȃ��ꍇ
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(...);

		//offset��0�ɂ���
		void reset() noexcept {
			m_offset = 0;
		}

		//�|�C���^�̎擾
		ID3D12DescriptorHeap*& get() noexcept {
			return m_descriptor_heap;
		}

		//�擪����stride�~num���i�񂾃n���h���̈ʒu
		//0�X�^�[�g
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0) {
			auto gpuHandle = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			gpuHandle.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(num);
			return gpuHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(unsigned int num = 0) {
			auto cpuHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			cpuHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(num);
			return cpuHandle;
		}
	};


	//
	//�N���X�e���v���[�g�̎���
	//

	template<typename DescriptorHeapType>
	inline bool descriptor_heap<DescriptorHeapType>::initialize(device* d, unsigned int size)
	{
		m_descriptor_heap = DescriptorHeapType::initialize(d, size);
		if (!m_descriptor_heap) {
			std::cout << "descriptor heap init is failed\n";
			return false;
		}
		m_increment_size = DescriptorHeapType::get_increment_size(d);
		m_size = size;

		return true;
	}


	template<typename DescriptorHeapType>
	template<typename CreateType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> 
		descriptor_heap<DescriptorHeapType>::create_view(device* device, ID3D12Resource* resource)
	{
		//�󂢂Ă�X�y�[�X���Ȃ��ꍇ
		if (m_offset >= m_size)
			return std::nullopt;

		//cpu�n���h���̎擾
		auto cpuHandle = get_cpu_handle(m_offset);

		//view�̐���
		//create_view_func<DescriptorHeapType,CreateType>(device,resource,cpuHandle)�̍쐻
		if (!create_view_func<DescriptorHeapType,CreateType>(device, resource, cpuHandle))
			return std::nullopt;

		//�߂�l�p��gpu�n���h���̎擾
		auto gpuHandle = get_gpu_handle(m_offset);

		//�I�t�Z�b�g�̍X�V
		m_offset++;

		return std::make_pair(gpuHandle, cpuHandle);
	}

	template<class DescriptorHeapType>
	template<typename T, typename CreateType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> ichi::descriptor_heap<DescriptorHeapType>::create_view(device* device, T* resourcePtr)
	{
		return create_view<CreateType>(device, resourcePtr->get());
	}

	template<class DescriptorHeapType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> ichi::descriptor_heap<DescriptorHeapType>::create_view(...)
	{
		std::cout << "descriptor heap create view failed type\n";
		return std::nullopt;
	}

	//
	//�f�B�X�N���v�^�[�̃^�C�v�p�̃p�����[�^�̃N���X�̒�`
	//


	//�萔�o�b�t�@�ƃV�F�[�_���\�[���Ƒ��P�p
	struct descriptor_heap_type::CBV_SRV_UAV
	{
		//�Ƃ肠�������ׂẴV�F�[�_���猩����悤�ɂ��Ă���
		//��ŃJ�X�^���ł���悤�ɂȂ邩��
		static ID3D12DescriptorHeap* initialize(device* device,int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = size;
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	
			if (FAILED(device->get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&result))))
				return nullptr;
			else
				return result;
		}

		static unsigned int get_increment_size(device* device)
		{
			return device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	};


	//�f�B�v�X�p
	struct descriptor_heap_type::DSV
	{
		//��������Ƃ肠�������ׂẴV�F�[�_���猩����悤��
		static ID3D12DescriptorHeap* initialize(device* device, int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			dsvHeapDesc.NodeMask = 0;
			dsvHeapDesc.NumDescriptors = size;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			if (FAILED(device->get()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&result))))
				return nullptr;
			else
				return result;
		}

		static unsigned int get_increment_size(device* device)
		{
			device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}
	};



	//
	//View�����֐��̒�`
	//

	//�f�t�H
	template<typename DescriptorHeapType,typename CreateType>
	inline bool create_view_func(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		return false;
	}


	//�f�B�X�N���v�^�q�[�v�̃^�C�v��CBV_SRV_UAV�̎���CBV�̃r���[�����
	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::CBV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::SRV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::DSV>
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
	inline bool  create_view_func<descriptor_heap_type::DSV, create_view_type::DSV>
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


	/*

	//
	//View����鎞�̃p�����[�^�p�̃N���X�̒�`
	//

	//�f�t�H
	template<typename DescriptorHeapType>
	struct create_view_type::CBV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};
	//�f�B�X�N���v�^�q�[�v�̃^�C�v��CBV_SRV_UAV�̎�
	template<>
	struct create_view_type::CBV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		{
			//�萔�o�b�t�@
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);

			//�萔�o�b�t�@�r���[�̍쐬
			device->get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

			return true;
		}
	};
	

	//�f�t�H
	template<typename DescriptorHeapType>
	struct create_view_type::SRV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};

	template<>
	struct create_view_type::SRV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
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
	};

	//�f�B�v�X
	//�f�t�H
	template<typename DescriptorHeapType>
	struct create_view_type::DSV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};
	//�[�x�o�b�t�@���V�F�[�_���\�[�X�Ƃ��Ĉ�������
	//CBV_SRV_UAV�̃f�B�X�N���v�^�q�[�v��View�����p
	template<>
	struct create_view_type::DSV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
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
		}
	};

	//�[�x�o�b�t�@��[�x�o�b�t�@�Ƃ��Ďg������
	//�[�x�o�b�t�@�p�̃f�B�X�N���v�^�q�[�v��View�����p
	template<>
	struct create_view_type::DSV<descriptor_heap_type::DSV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		{
			//�[�x�r���[�쐬
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�

			device->get()->CreateDepthStencilView(resource, //�r���[�Ɗ֘A�t����o�b�t�@
				&dsvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
				cpuHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
			);

			return false;
		}
	};
	*/


	/*

	//ConstantBuffer�p��ShaderResouce�p
	//sapmer��static��RootSignature�Őݒ�
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);

		//�r���[�̍쐻
		template<typename ResourceType>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, ID3D12Resource* resourcePtr);
		//�f�B�X�N���v�^�q�[�v���Z�b�g����悤��gpu_handle���擾�ł���悤�ɂ��Ă���
		//���\�[�X�^�C�v�������Ă���ꍇ�ɌĂ΂��֐�
		template<typename T,typename = typename T::resource_type>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, T* resourcePtr);
		//�����Ă��Ȃ��ꍇ
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(...);

		//offset��0�ɂ���
		void reset() noexcept;

		ID3D12DescriptorHeap*& get_ptr() noexcept;

		//�擪����stride�~num���i�񂾃n���h���̈ʒu
		//0�X�^�[�g
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0);
	};


	template<typename ResourceType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> descriptor_heap::create_view(device* device, ID3D12Resource* resourcePtr)
	{
		if (!resourcePtr)
			return std::nullopt;

		if (m_size <= m_offset)
			return std::nullopt;

		//�萔�o�b�t�@�̏ꍇ
		if constexpr (std::is_same_v<ResourceType, constant_buffer_tag>)
		{
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//�萔�o�b�t�@
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resourcePtr->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->GetDesc().Width);

			//�萔�o�b�t�@�r���[�̍쐬
			device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

			//�߂�l�p
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//�X�V
			m_offset++;

			return std::make_pair(h, heapHandle);
		}
		else if constexpr (std::is_same_v<ResourceType, shader_resource_tag>)
		{
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//�e�N�X�`��
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = resourcePtr->GetDesc().Format;//RGBA(0.0f�`1.0f�ɐ��K��)
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
			srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

			device->get()->CreateShaderResourceView(resourcePtr, //�r���[�Ɗ֘A�t����o�b�t�@
				&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
				heapHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
			);

			//�߂�l�p
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//�X�V
			m_offset++;

			return std::make_pair(h, heapHandle);
		}
		else if constexpr (std::is_same_v<ResourceType, depth_buffer_tag>)
		{
	
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//�e�N�X�`��
			D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
			resDesc.Format = DXGI_FORMAT_R32_FLOAT;
			resDesc.Texture2D.MipLevels = 1;
			resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

			device->get()->CreateShaderResourceView(resourcePtr, //�r���[�Ɗ֘A�t����o�b�t�@
				&resDesc, //��قǐݒ肵���e�N�X�`���ݒ���
				heapHandle//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
			);

			//�߂�l�p
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//�X�V
			m_offset++;

			return std::make_pair(h, heapHandle);
		}

		std::cout << "create view is failed\n";

		return std::nullopt;
	}


	template<typename T,typename ResourceType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> descriptor_heap::create_view(device* device, T* resourcePtr)
	{
		//std::cout << typeid(ResourceType).name() << "\n";
		return create_view<ResourceType>(device, resourcePtr->get());

	}

	*/

}