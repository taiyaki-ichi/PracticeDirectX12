#pragma once
#include"resource_state.hpp"

namespace DX12
{
	template<typename Format>
	class frame_buffer_resource
	{
		release_unique_ptr<ID3D12Resource> resource_ptr{};
		resource_state state{};

		std::optional<D3D12_CLEAR_VALUE> clear_value{};

	public:
		frame_buffer_resource() = default;
		~frame_buffer_resource() = default;

		void initialize(ID3D12Resource*);

		frame_buffer_resource(frame_buffer_resource&&) = default;
		frame_buffer_resource& operator=(frame_buffer_resource&&) = default;

		ID3D12Resource* get();

		D3D12_CLEAR_VALUE* get_clear_value();

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		using format = Format;
	};

	//
	//
	//

	template<typename Format>
	inline void frame_buffer_resource<Format>::initialize(ID3D12Resource* r)
	{
		resource_ptr.reset(r);
	}

	template<typename Format>
	inline ID3D12Resource* frame_buffer_resource<Format>::get()
	{
		return resource_ptr.get();
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