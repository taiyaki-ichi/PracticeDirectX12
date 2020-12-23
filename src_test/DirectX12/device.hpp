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
	class constant_resource;
	class texture2D_resource;
	class double_buffer;
	class pipeline_state;

	class device
	{
		ID3D12Device* m_device = nullptr;

		//deveice���̂ɕK�v
		//6�Ƃ�1�Ƃ��̐����͂��܂����������Ă��Ȃ�
		IDXGIFactory6* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();

		//constant_resource* create_constant_resource(unsigned int size);
		//texture2D_resource* create_texture2D_resource(unsigned int size);
		
		template<typename T,typename... Args>
		T* create(Args... args) {
			//�f�t�H���g�\�z���\
			static_assert(std::is_default_constructible_v<T>);
			//�����o�֐���initialize�������Ă��肻�̑�������device
			static_assert(std::is_invocable_v<decltype(&T::initialize), T, ichi::device*, Args...>);

			auto t = new T{};
			if (t->initialize(this, std::forward<Args>(args)...)) {
				return t;
			}
			else {
				std::cout << "device create is failed ";
				delete t;
				return nullptr;
			}
		
		}
		//std::is_invocable<decltype(&has_member::member_function), has_member*, std::vector<char>>:
		
		ID3D12Device* get();

	};
}