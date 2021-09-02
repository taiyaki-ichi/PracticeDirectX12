#pragma once
#include"../device.hpp"
#include"../format.hpp"
#include<optional>
#include<variant>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;

	enum class resource_heap_property
	{
		//とりあえず
		Default = D3D12_HEAP_TYPE_DEFAULT,
		Upload = D3D12_HEAP_TYPE_UPLOAD,
		ReadBack = D3D12_HEAP_TYPE_READBACK,
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
		Buffer = D3D12_RESOURCE_DIMENSION_BUFFER,
		//基本的にはこっち
		//LayoutはUnknowになる
		Texture2D = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
	};

	//
	template<resource_dimention>
	inline D3D12_TEXTURE_LAYOUT get_texture_layout()
	{
		static_assert(false);
	}
	template<>
	inline D3D12_TEXTURE_LAYOUT get_texture_layout<resource_dimention::Buffer>()
	{
		return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	}
	template<>
	inline D3D12_TEXTURE_LAYOUT get_texture_layout<resource_dimention::Texture2D>()
	{
		return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	}




	template<component_type ComponentType, std::uint32_t ComponentSize, std::uint8_t ComponentNum>
	struct format {
		static constexpr component_type component_type = ComponentType;
		static constexpr std::uint32_t component_size = ComponentSize;
		static constexpr std::uint32_t component_num = ComponentNum;
	};

	struct unknow_format {
		//
		static constexpr std::uint32_t component_size = 0;
		static constexpr std::uint32_t component_num = 0;
	};


	template<typename Format>
	inline constexpr std::optional<DXGI_FORMAT> get_resource_format()
	{
		if constexpr (std::is_same_v<Format, unknow_format>)
			return DXGI_FORMAT_UNKNOWN;
		else
			return get_dxgi_format(component_type::TYPELSEE, Format::component_size, Format::component_num);
	}


	enum class resource_state {
		Common = D3D12_RESOURCE_STATE_COMMON,
		RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
		PixcelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
		DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
		UnorderedAccessResource = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	};

	using clear_type_variant = std::variant<std::array<float, 3>, float>;

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag... Flags>
	class resource {

		release_unique_ptr<ID3D12Resource> resource_ptr{};

		resource_state state{};

		//
		std::optional<std::array<float, Format::component_num>> clear_value{};

	public:
		resource() = default;
		virtual ~resource() = default;

		resource(resource&&) = default;
		resource& operator=(resource&&) = default;

		//claerValueは仮
		void initialize(device&, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize,
			std::uint16_t mipLevels, std::optional<std::array<float, Format::component_num>> clearValue = std::nullopt);

		ID3D12Resource* get();

		D3D12_CLEAR_VALUE* get_clear_value();

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		static constexpr resource_dimention dimention = ResourceDimention;
		using format = Format;
		static constexpr resource_heap_property heap_property = HeapProperty;
		static constexpr D3D12_RESOURCE_FLAGS flags = []() {
			if constexpr (sizeof...(Flags) > 0) return (static_cast<D3D12_RESOURCE_FLAGS>(Flags) | ...);
			else return D3D12_RESOURCE_FLAG_NONE;
		}();


		//AllowDepthStencilとAllowRenderTargetを2つとも指定することはできない
		static_assert(!(flags& static_cast<D3D12_RESOURCE_FLAGS>(resource_flag::AllowDepthStencil) &&
			flags& static_cast<D3D12_RESOURCE_FLAGS>(resource_flag::AllowRenderTarget)));
	};


	//
	//
	//

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag ...Flags>
	inline void resource<ResourceDimention, Format, HeapProperty, Flags...>::initialize(device& device,
		std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, std::optional<std::array<float, Format::component_num>> clearValue)
	{
		//とりあえず
		D3D12_HEAP_PROPERTIES heap_prop{};
		heap_prop.Type = static_cast<D3D12_HEAP_TYPE>(HeapProperty);
		heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		

		D3D12_RESOURCE_DESC res_desc{};
		res_desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(ResourceDimention);
		res_desc.Width = width;
		res_desc.Height = height;
		res_desc.DepthOrArraySize = depthOrArraySize;
		res_desc.MipLevels = mipLevels;
		res_desc.Format = get_resource_format<Format>().value();
		res_desc.SampleDesc.Count = 1;
		res_desc.Flags = flags;
		res_desc.Layout = get_texture_layout<ResourceDimention>();

		clear_value = clearValue;
		D3D12_CLEAR_VALUE cv{};
		if (clearValue)
		{
			if constexpr (flags & static_cast<D3D12_RESOURCE_FLAGS>(resource_flag::AllowDepthStencil)) {
				cv.Format = get_depth_stencil_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();
				cv.DepthStencil.Depth = clearValue.value()[0];
			}

			if constexpr (flags & static_cast<D3D12_RESOURCE_FLAGS>(resource_flag::AllowRenderTarget)) {
				cv.Format = get_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();
				std::copy(clearValue.value().begin(), clearValue.value().end(), std::begin(cv.Color));
			}
		}

		//とりあえず
		if constexpr (HeapProperty == resource_heap_property::Upload)
			state = resource_state::GenericRead;
		else if constexpr (HeapProperty == resource_heap_property::ReadBack)
			state = resource_state::CopyDest;
		else
			state = resource_state::Common;

		ID3D12Resource* tmp = nullptr;
		if (FAILED(device.get()->CreateCommittedResource(
			&heap_prop,
			D3D12_HEAP_FLAG_NONE,
			&res_desc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			(clearValue) ? &cv : nullptr,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_EXCEPTION("");
		}
		resource_ptr.reset(tmp);
	}

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag ...Flags>
	inline ID3D12Resource* resource<ResourceDimention, Format, HeapProperty, Flags...>::get()
	{
		return resource_ptr.get();
	}

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag ...Flags>
	inline D3D12_CLEAR_VALUE* resource<ResourceDimention, Format, HeapProperty, Flags...>::get_clear_value()
	{
		if (clear_value)
			return &clear_value.value();
		else
			return nullptr;
	}

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag ...Flags>
	inline resource_state resource<ResourceDimention, Format, HeapProperty, Flags...>::get_state() const noexcept
	{
		return state;
	}

	template<resource_dimention ResourceDimention, typename Format, resource_heap_property HeapProperty, resource_flag ...Flags>
	inline void resource<ResourceDimention, Format, HeapProperty, Flags...>::set_state(resource_state s) noexcept
	{
		state = s;
	}
}