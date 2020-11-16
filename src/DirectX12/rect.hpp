#pragma once
#include<DirectXMath.h>
#include<array>
#include<memory>
#include<d3d12.h>

namespace graphics
{
	class rect
	{
	public:
		rect(ID3D12Device* device,float window_width,float window_height);

		//描写
		void draw_command(ID3D12GraphicsCommandList*);

		//セッタ
		void set_RGBA(float, float, float, float);
		void set_width_and_height(float,float);
		void set_rotation(float);

	private:

		//変換用の行列
		DirectX::XMMATRIX m_matrix;

		ID3D12DescriptorHeap* m_descriptor_heap;

		ID3D12RootSignature* m_root_signature;

		//頂点シェーダ
		ID3DBlob* m_vert_shader_blob;
		//ピクセルシェーダ
		ID3DBlob* m_pixcel_shader_blob;

		//頂点とインデックスのリソース
		ID3D12Resource* m_vertex_buffer;
		ID3D12Resource* m_index_buffer;
		
		//リソース
		ID3D12Resource* m_constant_matrix_buffer;
		ID3D12Resource* m_constant_color_buffer;
		//ビュー
		D3D12_VERTEX_BUFFER_VIEW m_vertex_view;
		D3D12_INDEX_BUFFER_VIEW m_index_view;

		//パイプラインステート
		ID3D12PipelineState* m_graphics_pipeline_state;
	};
}