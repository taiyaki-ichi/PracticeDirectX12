#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace dev
{
	class Device;

	enum class resource_heap_property
	{
		Default,
		Upload,
		ReadBack,
	};

	enum class resource_flag
	{
		AllowRenderTarget = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		AllowDepthStencil = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		AllowUnorderdAccess = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		DenyShederResource = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
	};

	enum class resource_dimention
	{
		//Mapする時などに使用
		//この場合LayoutはD3D12_TEXTURE_LAYOUT_ROW_MAJOR
		Buffer,
		//基本的にはこっち
		//LayoutはUnknowになる
		Texture2D,
	};

	enum class Type
	{
		Float16,
		Float32,
		Uint16,
		Uint32,
		UnsignedNormalizedFloat8,
		Typeless16,
		Typeless32
	};

	template<Type T, std::uint8_t S>
	struct Format {
		using type = T;
		static constexpr std::uint8_t size = S;
	};

	enum class resource_state {
		Common = D3D12_RESOURCE_STATE_COMMON,
		RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
		PixcelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
		DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
		UnorderedAccessResource = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	};


	template<resource_dimention ResourceDimention,typename Format,resource_heap_property HeapProperty,resource_flag... Flags>
	class resource {

		ID3D12Resource* resource = nullptr;

		resource_state state{};
		std::optional<D3D12_CLEAR_VALUE> clearValue{};

	public:
		void Initialize(Device*, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels);

		ID3D12Resource* Get();

		D3D12_CLEAR_VALUE* GetClearValue();

		const resource_state GetState() const noexcept;
		void SetState(resource_state) noexcept;

		static constexpr resource_dimention dimention = ResourceDimention;
		using format = Format;
		static constexpr resource_heap_property = HeapProperty;
		static constexpr D3D12_RESOURCE_FLAGS flags = []() {
			if constexpr (sizeof...(Nums) > 0) return static_cast<D3D12_RESOURCE_FLAGS>(flags) + ...;
			else return D3D12_RESOURCE_FLAG_NONE;
		}();
	};



}