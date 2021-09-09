#pragma once
#include"buffer_resource.hpp"
#include"mapped_resource.hpp"

namespace DX12
{
	//テクスチャをアップロードするよう
	template<typename Format>
	class texture_upload_buffer_resource : public buffer_resource<resource_heap_property::Upload>
	{
		std::uint32_t width = 0;

	public:
		void initialize(device& device, std::uint32_t width, std::uint32_t height);

		std::uint32_t get_width() const noexcept;

		using mapped_resource_type = texture_upload_mapped_resource<Format>;
	};

	//
	//
	//

	template<typename Format>
	void texture_upload_buffer_resource<Format>::initialize(device& device, std::uint32_t w, std::uint32_t h)
	{
		width = alignment<std::uint32_t>(w * get_format_stride<Format>(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		buffer_resource<resource_heap_property::Upload>::initialize(device, width * h * get_format_stride<Format>());
	}

	template<typename Format>
	inline std::uint32_t texture_upload_buffer_resource<Format>::get_width() const noexcept
	{
		return width;
	}

}