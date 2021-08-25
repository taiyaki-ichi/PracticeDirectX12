#pragma once
#include"Device.hpp"
#include"View.hpp"
#include<utility>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class Device;

	class descriptor_heap_base
	{
		ID3D12DescriptorHeap* descriptor_heap = nullptr;

		std::uint32_t size = 0;
		std::uint32_t offset = 0;
		std::uint32_t increment_size = 0;

	public:
		descriptor_heap_base() = default;
		virtual ~descriptor_heap_base();

		descriptor_heap_base(const descriptor_heap_base&) = delete;
		descriptor_heap_base& operator=(const descriptor_heap_base&) = delete;

		descriptor_heap_base(descriptor_heap_base&&) noexcept;
		descriptor_heap_base& operator=(descriptor_heap_base&&) noexcept;

		void initialize(Device* device, std::uint32_t size, D3D12_DESCRIPTOR_HEAP_DESC desc, std::uint32_t incrementSize);

		template<typename Resource, typename CreateViewFunc, typename... CreateViewFuncOptionArgs>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>
			push_back_view(Device* device, Resource* resource, CreateViewFunc createViewFunc, CreateViewFuncOptionArgs&&... createViewPolicyArgs);

		ID3D12DescriptorHeap*& get() noexcept;

		D3D12_GPU_DESCRIPTOR_HANDLE get_GPU_handle(std::size_t index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE get_CPU_handle(std::size_t index = 0);
	};


	class descriptor_heap_CBV_SRV_UAV : public descriptor_heap_base
	{
	public:
		void initialize(Device* device, std::uint32_t size);


		template<typename Resource>
		void push_back_CBV(Device*, Resource*, std::uint32_t sizeInBytes);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_SRV(Device*, Resource*,
			std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_array_SRV(Device*, Resource*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture_cube_array_SRV(Device*, Resource*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_UAV(Device*, Resource*,std::uint32_t mipSlice, std::uint32_t planeSlice);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_array_UAV(Device*, Resource*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice);

	};

	class descriptor_heap_DSV : public descriptor_heap_base
	{
	public:
		void initialize(Device* device, std::uint32_t size);

		template<typename Resource>
		void push_back_textre2D_DSV(Device*, Resource*, std::uint32_t mipSlice);

		template<typename Resource>
		void push_back_textre2D_array_DSV(Device*, Resource*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice);
	};

	class descriptor_heap_RTV : public descriptor_heap_base
	{
	public:
		void initialize(Device* device, std::uint32_t size);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_RTV(Device*, Resource*, std::uint32_t mipSlice, std::uint32_t planeSlice);

		template<component_type ViewComponentType, typename Resource>
		void push_back_texture2D_array_RTV(Device*, Resource*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice);
	};


	//
	//
	//

	DX12::descriptor_heap_base::~descriptor_heap_base()
	{
		if (descriptor_heap)
			descriptor_heap->Release();
	}

	inline descriptor_heap_base::descriptor_heap_base(descriptor_heap_base&& rhs) noexcept
	{
		descriptor_heap = rhs.descriptor_heap;
		size = rhs.size;
		offset = rhs.offset;
		increment_size = rhs.increment_size;

		rhs.descriptor_heap = nullptr;
	}

	inline descriptor_heap_base& descriptor_heap_base::operator=(descriptor_heap_base&& rhs) noexcept
	{
		descriptor_heap = rhs.descriptor_heap;
		size = rhs.size;
		offset = rhs.offset;
		increment_size = rhs.increment_size;

		rhs.descriptor_heap = nullptr;

		return *this;
	}

	inline void descriptor_heap_base::initialize(Device* device, std::uint32_t s, D3D12_DESCRIPTOR_HEAP_DESC desc, std::uint32_t incrementSize)
	{
		if (FAILED(device->Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap))))
			throw "";

		size = s;
		increment_size = incrementSize;
	}

	template<typename Resource, typename CreateViewFunc, typename ...CreateViewFuncOptionArgs>
	inline std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> descriptor_heap_base::push_back_view(Device* device, Resource* resource, CreateViewFunc createViewFunc, CreateViewFuncOptionArgs && ...createViewPolicyArgs)
	{
		//空いてるスペースがない場合
		if (offset >= size)
			throw "";

		//cpuハンドルの取得
		auto cpuHandle = get_CPU_handle(offset);

		//viewの生成
		//
		std::invoke(createViewFunc, device->Get(), resource->get(), cpuHandle, std::forward<CreateViewFuncOptionArgs>(createViewPolicyArgs)...);

		//戻り値用にgpuハンドルの取得
		auto gpuHandle = get_GPU_handle(offset);

		//オフセットの更新
		offset++;

		return std::make_pair(gpuHandle, cpuHandle);
	}

	inline ID3D12DescriptorHeap*& descriptor_heap_base::get() noexcept
	{
		return descriptor_heap;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap_base::get_GPU_handle(std::size_t index)
	{
		auto gpuHandle = descriptor_heap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += static_cast<UINT64>(increment_size) * static_cast<UINT64>(index);
		return gpuHandle;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE DX12::descriptor_heap_base::get_CPU_handle(std::size_t index)
	{
		auto cpuHandle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += static_cast<SIZE_T>(increment_size) * static_cast<SIZE_T>(index);
		return cpuHandle;
	}

	inline void descriptor_heap_CBV_SRV_UAV::initialize(Device* device, std::uint32_t size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		auto incrementSize = device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		descriptor_heap_base::initialize(device, size, desc, incrementSize);
	}

	template<typename Resource>
	inline void descriptor_heap_CBV_SRV_UAV::push_back_CBV(Device* device, Resource* resource, std::uint32_t sizeInBytes)
	{
		push_back_view(device, resource, create_CBV, sizeInBytes);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_CBV_SRV_UAV::push_back_texture2D_SRV(Device* device, Resource* resource, 
		std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp)
	{
		static_assert(!(Resource::flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE));
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		push_back_view(device, resource, create_texture2D_SRV<ViewComponentType, Resource::typeless_format>, mipLevels, mostDetailedMip, planeSline, resourceMinLODClamp);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void DX12::descriptor_heap_CBV_SRV_UAV::push_back_texture2D_array_SRV(Device* device, Resource* resource,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		static_assert(!(Resource::flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE));
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		push_back_view(device, resource, create_texture2D_array_SRV<ViewComponentType, Resource::typeless_format>,
			arraySize, firstArraySlice, mipLevels, mostDetailedMip, planeSlice, resourceMinLODClamp);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_CBV_SRV_UAV::push_back_texture_cube_array_SRV(Device* device, Resource* resource,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		static_assert(!(Resource::flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE));
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		push_back_view(device, resource, create_texture_cube_SRV<ViewComponentType, Resource::typeless_format>,
			arraySize, firstArraySlice, mipLevels, mostDetailedMip, planeSlice, resourceMinLODClamp);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_CBV_SRV_UAV::push_back_texture2D_UAV(Device* device, Resource* resource, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		//とりあえずCounterResourceはnullptrで
		push_back_view(device, resource, create_texture2D_UAV<ViewComponentType, Resource::typeless_format>, nullptr, mipSlice, planeSlice);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_CBV_SRV_UAV::push_back_texture2D_array_UAV(Device* device, Resource* resource, 
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		//とりあえずCounterResourceはnullptrで
		push_back_view(device, resource, create_texture2D_array_UAV<ViewComponentType, Resource::typeless_format>,
			nullptr, arraySize, firstArraySlice, mipSlice, planeSlice);
	}

	inline void descriptor_heap_DSV::initialize(Device* device, std::uint32_t size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		auto incrementSize = device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		descriptor_heap_base::initialize(device, size, desc, incrementSize);
	}

	template<typename Resource>
	inline void descriptor_heap_DSV::push_back_textre2D_DSV(Device* device, Resource* resource, std::uint32_t mipSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		//ここのフォーマットのstaticassert

		push_back_view(device, resource, create_texture2D_DSV<Resource::typeless_format>, mipSlice);
	}

	template<typename Resource>
	inline void descriptor_heap_DSV::push_back_textre2D_array_DSV(Device* device, Resource* resource, 
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		//
		//

		push_back_view(device, resource, create_texture2D_array_DSV<Resource::typeless_format>, arraySize, firstArraySlice, mipSlice);
	}

	void descriptor_heap_RTV::initialize(Device* device, std::uint32_t size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		auto incrementSize = device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		descriptor_heap_base::initialize(device, size, desc, incrementSize);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_RTV::push_back_texture2D_RTV(Device* device, Resource* resource, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		push_back_view(device, resource, create_texture2D_RTV<ViewComponentType, Resource::typeless_format>, mipSlice, planeSlice);
	}

	template<component_type ViewComponentType, typename Resource>
	inline void descriptor_heap_RTV::push_back_texture2D_array_RTV(Device* device, Resource* resource, 
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		static_assert(Resource::flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		static_assert(get_dxgi_format(ViewComponentType, Resource::typeless_format::component_size, Resource::typeless_format::component_num));

		push_back_view(device, resource, create_texture2D_array_RTV<ViewComponentType, Resource::typeless_format>, 
			arraySize, firstArraySlice, mipSlice, planeSlice);
	}
}