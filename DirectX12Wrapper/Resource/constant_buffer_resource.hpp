#pragma once
#include"buffer_resource.hpp"
#include"../utility.hpp"

namespace DX12
{
	template<typename T>
	class constant_buffer_resource : public buffer_resource
	{
	public:
		void initialize(device& device);

		std::uint32_t get_size() const noexcept;
	};

	//
	//
	//

	template<typename T>
	void constant_buffer_resource<T>::initialize(device& device)
	{
		buffer_resource::initialize(device, alignment<std::uint32_t>(sizeof(T), 256));
	}

	template<typename T>
	inline std::uint32_t constant_buffer_resource<T>::get_size() const noexcept
	{
		return alignment<std::uint32_t>(sizeof(T), 256);
	}
}