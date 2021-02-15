#include"float_shader_resource.hpp"

namespace DX12
{

	bool float_shader_resource::initialize(device* device, unsigned int width, unsigned int height, std::optional<D3D12_CLEAR_VALUE> clearValue)
	{
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
		resdesc.Width = width;//幅と高さはレンダーターゲットと同じ
		resdesc.Height = height;//上に同じ
		resdesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
		resdesc.Format = DXGI_FORMAT_R32_FLOAT;
		resdesc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_CLEAR_VALUE* clear = nullptr;
		if (clearValue)
			clear = &clearValue.value();

		return resource_base::initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //デプス書き込みに使用
			clear
		);
	}
}