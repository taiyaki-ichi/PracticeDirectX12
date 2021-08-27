#pragma once
#include"resource.hpp"

namespace DX12
{
	template<typename Format>
	class frame_buffer_resource
	{
		ID3D12Resource* resource_ptr = nullptr;
		resource_state state{};

		std::optional<D3D12_CLEAR_VALUE> clear_value{};

	public:
		frame_buffer_resource() = default;
		~frame_buffer_resource();

		void initialize(ID3D12Resource*);

		frame_buffer_resource(frame_buffer_resource&) = delete;
		frame_buffer_resource& operator=(frame_buffer_resource&) = delete;

		frame_buffer_resource(frame_buffer_resource&&) noexcept;
		frame_buffer_resource& operator=(frame_buffer_resource&&) noexcept;

		ID3D12Resource* get();

		D3D12_CLEAR_VALUE* get_clear_value();

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		static constexpr resource_dimention dimention = resource_dimention::Texture2D;
		using format = Format;
		static constexpr resource_heap_property heap_property = resource_heap_property::Default;//
		static constexpr D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;//
	};

	//
	//
	//

	template<typename Format>
	inline frame_buffer_resource<Format>::~frame_buffer_resource()
	{
		if (resource_ptr)
			resource_ptr->Release();
	}

	template<typename Format>
	inline void frame_buffer_resource<Format>::initialize(ID3D12Resource* r)
	{
		resource_ptr = r;
	}

	template<typename Format>
	inline frame_buffer_resource<Format>::frame_buffer_resource(frame_buffer_resource&& rhs) noexcept
	{
		resource_ptr = rhs.resource_ptr;
		state = rhs.state;
		clear_value = std::move(rhs.clear_value);

		rhs.resource_ptr = nullptr;
	}

	template<typename Format>
	inline frame_buffer_resource<Format>& frame_buffer_resource<Format>::operator=(frame_buffer_resource&& rhs) noexcept
	{
		resource_ptr = rhs.resource_ptr;
		state = rhs.state;
		clear_value = std::move(rhs.clear_value);

		rhs.resource_ptr = nullptr;

		return *this;
	}

	template<typename Format>
	inline ID3D12Resource* frame_buffer_resource<Format>::get()
	{
		return resource_ptr;
	}

	template<typename Format>
	inline D3D12_CLEAR_VALUE* frame_buffer_resource<Format>::get_clear_value()
	{
		return clear_value;
	}

	template<typename Format>
	inline resource_state frame_buffer_resource<Format>::get_state() const noexcept
	{
		return state;
	}

	template<typename Format>
	inline void frame_buffer_resource<Format>::set_state(resource_state s) noexcept
	{
		state = s;
	}

}