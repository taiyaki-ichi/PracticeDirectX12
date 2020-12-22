#pragma once
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

		//deveiceì‚é‚Ì‚É•K—v
		//6‚Æ‚©1‚Æ‚©‚Ì”š‚Í‚¢‚Ü‚¢‚¿—‰ğ‚µ‚Ä‚¢‚È‚¢
		IDXGIFactory6* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();

		command_list* create_command_list();
		double_buffer* create_double_buffer(HWND,command_list*);
		pipeline_state* create_pipline_state(ID3DBlob* vertexShader, ID3DBlob* pixelShader);
	
		//vertex_buffer* create_vertex_buffer(unsigned int size);
		//index_buffer* create_index_buffer(unsigned int size);
		//constant_resource* create_constant_resource(unsigned int size);
		//texture2D_resource* create_texture2D_resource(unsigned int size);
		
		
		ID3D12Device* get();

	};
}