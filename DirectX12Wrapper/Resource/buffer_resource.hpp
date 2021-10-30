#pragma once
#include"resource_state.hpp"

namespace DX12
{
	enum class resource_heap_property
	{
		DEFAULT = D3D12_HEAP_TYPE_DEFAULT,
		UPLOAD = D3D12_HEAP_TYPE_UPLOAD,
		READ_BACK = D3D12_HEAP_TYPE_READBACK,
	};

	template<typename FormatOrFormatTupleOrStruct, resource_heap_property HeapProperty>
	class buffer_resource
	{
		release_unique_ptr<ID3D12Resource> resource_ptr{};
		resource_state state{};

		std::uint32_t num{};

	public:
		void initialize(device&, std::uint32_t n);

		ID3D12Resource* get() noexcept;

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		std::uint32_t get_num() const noexcept;

		using value_type = FormatOrFormatTupleOrStruct;
		static constexpr resource_heap_property heap_property = HeapProperty;
	};

	//
	//
	//


	template<typename ValueType, resource_heap_property HeapProperty>
	inline void buffer_resource<ValueType, HeapProperty>::initialize(device& d, std::uint32_t n)
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = static_cast<D3D12_HEAP_TYPE>(HeapProperty);
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		std::uint32_t width{};
		if constexpr (is_format_tuple<ValueType>::value)
			width = ValueType::get_formats_stride() * n;
		else if constexpr (is_format<ValueType>::value)
			width = get_format_stride<ValueType>() * n;
		else
			width = alignment<std::uint32_t>(sizeof(ValueType) * n, 256);


		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		num = n;
		switch (HeapProperty)
		{
		case resource_heap_property::DEFAULT:
			state = resource_state::Common;
			break;
		case resource_heap_property::READ_BACK:
			state = resource_state::CopyDest;
			break;
		case resource_heap_property::UPLOAD:
			state = resource_state::GenericRead;
			break;
		default:
			break;
		}

		ID3D12Resource* tmp = nullptr;
		if (FAILED(d.get()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			nullptr,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_EXCEPTION("");
		}
		resource_ptr.reset(tmp);
	}

	template<typename FormatOrFormatTupleOrStruct, resource_heap_property HeapProperty>
	inline ID3D12Resource* buffer_resource<FormatOrFormatTupleOrStruct, HeapProperty>::get() noexcept
	{
		return resource_ptr.get();
	}

	template<typename FormatOrFormatTupleOrStruct, resource_heap_property HeapProperty>
	inline resource_state buffer_resource<FormatOrFormatTupleOrStruct, HeapProperty>::get_state() const noexcept
	{
		return state;
	}

	template<typename FormatOrFormatTupleOrStruct, resource_heap_property HeapProperty>
	inline void buffer_resource<FormatOrFormatTupleOrStruct, HeapProperty>::set_state(resource_state s) noexcept
	{
		state = s;
	}

	template<typename FormatOrFormatTupleOrStruct, resource_heap_property HeapProperty>
	inline std::uint32_t buffer_resource<FormatOrFormatTupleOrStruct, HeapProperty>::get_num() const noexcept
	{
		return num;
	}
}