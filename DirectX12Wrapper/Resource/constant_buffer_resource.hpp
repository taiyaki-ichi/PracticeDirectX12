#pragma once
#include"buffer_resource.hpp"
#include"../Utility.hpp"

namespace DX12
{
	template<typename T>
	class constant_buffer_resource : public buffer_resource
	{
	public:
		void initialize(Device* device);

		std::uint32_t get_size() const noexcept;
	};

	//
	//
	//

	template<typename T>
	void constant_buffer_resource<T>::initialize(Device* device)
	{
		buffer_resource::initialize(device, Alignment<std::uint32_t>(sizeof(T), 256));
	}

	template<typename T>
	inline std::uint32_t constant_buffer_resource<T>::get_size() const noexcept
	{
		return Alignment<std::uint32_t>(sizeof(T), 256);
	}
}