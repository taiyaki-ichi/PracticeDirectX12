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
	
	class DescriptorHeapBase
	{
		ID3D12DescriptorHeap* descriptorHeap = nullptr;

		std::uint32_t size = 0;
		std::uint32_t offset = 0;
		std::uint32_t incrementSize = 0;

	public:
		DescriptorHeapBase() = default;
		virtual ~DescriptorHeapBase();

		DescriptorHeapBase(const DescriptorHeapBase&) = delete;
		DescriptorHeapBase& operator=(const DescriptorHeapBase&) = delete;

		DescriptorHeapBase(DescriptorHeapBase&&) noexcept;
		DescriptorHeapBase& operator=(DescriptorHeapBase&&) noexcept;

		void Initialize(Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc, std::uint32_t incrementSize);

		template<typename ResourceType, typename CreateViewFunc, typename... CreateViewFuncOptionArgs>
		std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>
			PushBackView(Device* device, ResourceType* resource, CreateViewFunc createViewFunc, CreateViewFuncOptionArgs&&... createViewPolicyArgs);

		ID3D12DescriptorHeap*& Get() noexcept;

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(std::size_t index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(std::size_t index = 0);
	};


	class DescriptorHeap_CBV_SRV_UAV : public DescriptorHeapBase
	{
	public:
		void Initialize(Device* device, std::uint32_t size);


		template<typename ResourceType>
		void PushBackCBV(Device*, ResourceType*, std::uint32_t sizeInBytes);

		template<ViewComponentType VCT,typename ResourceType>
		void PushBackTextre2DSRV(Device*, ResourceType*,
			std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp);

		template<ViewComponentType VCT,typename ResourceType>
		void PushBackTexture2DArraySRV(Device*, ResourceType*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<ViewComponentType VCT, typename ResourceType>
		void PushBackTextreCubeArraySRV(Device*, ResourceType*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);

		template<ViewComponentType VCT, typename ResourceType>
		void PushBackUAV(Device*,ResourceType*,
			std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp);
	};

	class Descriptor_DSV : public DescriptorHeapBase
	{
	public:
		void Initialize(Device* device, std::uint32_t size);

		template<typename ResourceType>
		void PushBackTextre2DDSV(Device*, ResourceType*, std::uint32_t mipSlice);

		template<typename ResourceType>
		void PushBackTextre2DArrayDSV(Device*, ResourceType*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice);
	};

	class Descriptor_RTV : public DescriptorHeapBase
	{
	public:
		void Initialize(Device* device, std::uint32_t size);

		template<ViewComponentType VCT, typename ResourceType>
		void PushBackTextre2DRTV(Device*, ResourceType*, std::uint32_t mipSlice, std::uint32_t planeSlice);

		template<ViewComponentType VCT, typename ResourceType>
		void PushBackTextre2DArrayRTV(Device*, ResourceType*, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice);
	};
}