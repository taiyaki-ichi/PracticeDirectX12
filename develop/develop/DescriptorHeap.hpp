#pragma once
#include"View.hpp"
#include<utility>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace dev
{
	class Device;

	namespace DescriptorHeapTypeTag
	{
		struct CBV_SRV_UAV {};
		struct DSV {};
		struct RTV {};
	}
	
	class descriptor_heap_base
	{
		ID3D12DescriptorHeap* descriptorHeap = nullptr;

		std::uint32_t size = 0;
		std::uint32_t offset = 0;
		std::uint32_t incrementSize = 0;

	public:
		descriptor_heap_base() = default;
		virtual ~descriptor_heap_base();

		descriptor_heap_base(const descriptor_heap_base&) = delete;
		descriptor_heap_base& operator=(const descriptor_heap_base&) = delete;

		descriptor_heap_base(descriptor_heap_base&&) noexcept;
		descriptor_heap_base& operator=(descriptor_heap_base&&) noexcept;

		void Initialize(Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc, std::uint32_t incrementSize);

		template<typename resource, typename CreateViewFunc, typename... CreateViewFuncOptionArgs>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>
			PushBackView(Device* device, resource* resource, CreateViewFunc createViewFunc, CreateViewFuncOptionArgs&&... createViewPolicyArgs);

		ID3D12DescriptorHeap*& Get() noexcept;

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(std::size_t index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(std::size_t index = 0);
	};


	class descriptor_heap_CBV_SRV_UAV : public descriptor_heap_base
	{
	public:
		void Initialize(Device* device, std::uint32_t size);


		template<typename resource>
		void PushBackCBV(Device*, resource*, std::uint32_t sizeInBytes);

		template<component_type ViewComponentType,typename resource>
		void PushBackTextre2DSRV(Device*, resource*,
			std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp);

		template<component_type ViewComponentType,typename resource>
		void PushBackTexture2DArraySRV(Device*, resource*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<component_type ViewComponentType, typename resource>
		void PushBackTextreCubeArraySRV(Device*, resource*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<component_type ViewComponentType, typename resource>
		void PushBackUAV(Device*,resource*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);
	};

	class descriptor_heap_DSV : public descriptor_heap_base
	{
	public:
		void Initialize(Device* device, std::uint32_t size);

		template<typename resource>
		void PushBackTextre2DDSV(Device*, resource*, std::uint32_t mipSlice);

		template<typename resource>
		void PushBackTextre2DArrayDSV(Device*, resource*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice);
	};

	class descriptor_heap_RTV : public descriptor_heap_base
	{
	public:
		void Initialize(Device* device, std::uint32_t size);

		template<component_type ViewComponentType, typename resource>
		void PushBackTextre2DRTV(Device*, resource*, std::uint32_t mipSlice, std::uint32_t planeSlice);

		template<component_type ViewComponentType, typename resource>
		void PushBackTextre2DArrayRTV(Device*, resource*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice);
	};
}