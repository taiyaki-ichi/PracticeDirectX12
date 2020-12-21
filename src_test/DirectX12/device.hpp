#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class vertex_buffer;
	class index_buffer;
	class constant_resource;
	class texture2D_resource;
	class double_buffer;

	class device
	{
		ID3D12Device* m_device = nullptr;

		//deveiceçÏÇÈÇÃÇ…ïKóvÇ¡Ç€Ç¢
		IDXGIFactory6* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();

		vertex_buffer* create_vertex_buffer(unsigned int size);
		index_buffer* create_index_buffer(unsigned int size);
		constant_resource* create_constant_resource(unsigned int size);
		texture2D_resource* create_texture2D_resource(unsigned int size);
		double_buffer* create_double_buffer();
		
		ID3D12Device* get();

	};
}