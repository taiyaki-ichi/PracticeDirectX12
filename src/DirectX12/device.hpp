#pragma once
#include<type_traits>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class command_list;
	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	template<bool>
	class texture_shader_resource_base;
	class double_buffer;
	class pipeline_state;

	class device
	{
		ID3D12Device* m_device = nullptr;

		//deveice���̂ɕK�v
		//6�Ƃ�1�Ƃ��̐����͂��܂����������Ă��Ȃ�
		IDXGIFactory5* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();

		//device���g���č��n		
		template<typename T,typename... Args>
		T* create(Args&&... args) {
			//�f�t�H���g�\�z���\
			static_assert(std::is_default_constructible_v<T>);
			//�����o�֐���initialize�������Ă��肻�̑�������device
			static_assert(std::is_invocable_v<decltype(&T::initialize), T, ichi::device*, Args...>);

			auto t = new T{};
			if (t->initialize(this, std::forward<Args>(args)...)) {
				return t;
			}
			else {
				std::cout << "device create is failed\n";
				delete t;
				return nullptr;
			}
		
		}
		
		ID3D12Device* get() const noexcept;

	};
}