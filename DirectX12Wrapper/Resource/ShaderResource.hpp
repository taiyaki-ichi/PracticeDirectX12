#pragma once
#include"ResourceBase.hpp"
#include"DescriptorHeap/ViewTypeTag.hpp"
#include"../Format.hpp"
#include<array>
#include<variant>

namespace DX12
{

	class ShaderResource : public ResourceBase
	{
	public:
		void Initialize(Device*, std::uint32_t width, std::uint32_t height, Format,std::uint16_t depthOrArraySize);
		void Initialize(Device*, std::uint32_t width, std::uint32_t height, Format,std::uint16_t depthOrArraySize, std::array<float, 4> clearValue);
		void Initialize(Device*, std::uint32_t width, std::uint32_t height, Format, std::uint16_t depthOrArraySize, float depthClearValue);

	private:
		void InitializeImpl(Device*, std::uint32_t width, std::uint32_t height, Format, std::uint16_t depthOrArraySize, D3D12_CLEAR_VALUE*);
	};

	template<>
	struct DefaultViewTypeTraits<ShaderResource> {
		using Type = ViewTypeTag::ShaderResource;
	};

	//
	//
	//

	inline void ShaderResource::Initialize(Device* device, std::uint32_t width, std::uint32_t height, Format format, std::uint16_t depthOrArraySize)
	{
		InitializeImpl(device, width, height, format, depthOrArraySize, nullptr);
	}

	inline void ShaderResource::Initialize(Device* device, std::uint32_t width, std::uint32_t height, Format format, std::uint16_t depthOrArraySize, std::array<float, 4> clearValue)
	{
		D3D12_CLEAR_VALUE cv{};
		cv.Format = format.value;
		std::copy(std::begin(clearValue), std::end(clearValue), std::begin(cv.Color));
		InitializeImpl(device, width, height, format, depthOrArraySize, &cv);
	}

	inline void ShaderResource::Initialize(Device* device, std::uint32_t width, std::uint32_t height, Format format, std::uint16_t depthOrArraySize, float depthClearValue)
	{
		D3D12_CLEAR_VALUE cv{};
		cv.Format = format.value;
		cv.DepthStencil.Depth = depthClearValue;
		InitializeImpl(device, width, height, format, depthOrArraySize, &cv);
	}

	inline void ShaderResource::InitializeImpl(Device* device, std::uint32_t width, std::uint32_t height, Format format, std::uint16_t depthOrArraySize, D3D12_CLEAR_VALUE* clearValuePtr)
	{
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resdesc.Width = width;
		resdesc.Height = height;
		resdesc.Format = format.value;
		resdesc.DepthOrArraySize = depthOrArraySize;
		resdesc.SampleDesc.Count = 1;
		//
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		ResourceBase::Initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			ResourceState::PixcelShaderResource,
			clearValuePtr
		);
	}
}