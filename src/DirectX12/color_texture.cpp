#include"color_texture.hpp"
#include"device.hpp"
#include<vector>

namespace ichi
{

	namespace
	{
		//色のついていない4×4のテクスチャのリソースを返す
		ID3D12Resource* create_color_texture_base(device* device)
		{
			D3D12_HEAP_PROPERTIES texHeapProp{};
			texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
			texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
			texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
			texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
			texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

			D3D12_RESOURCE_DESC resDesc{};
			resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			resDesc.Width = 4;//幅
			resDesc.Height = 4;//高さ
			resDesc.DepthOrArraySize = 1;
			resDesc.SampleDesc.Count = 1;
			resDesc.SampleDesc.Quality = 0;//
			resDesc.MipLevels = 1;//
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
			resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

			ID3D12Resource* result = nullptr;
			if (SUCCEEDED(device->get()->CreateCommittedResource(
				&texHeapProp,
				D3D12_HEAP_FLAG_NONE,//特に指定なし
				&resDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				nullptr,
				IID_PPV_ARGS(&result)
			))) {
				return result;
			}
			else {
				return nullptr;
			}
		}
	}

	ID3D12Resource* create_white_texture(device* device)
	{

		ID3D12Resource* result = create_color_texture_base(device);
		if (!result) {
			std::cout << "create white texture is failed\n";
			return nullptr;
		}

		std::vector<unsigned char> data(4 * 4 * 4);
		std::fill(data.begin(), data.end(), 0xff);
		
		if (SUCCEEDED(result->WriteToSubresource(
			0,
			nullptr,
			data.data(),
			4 * 4,
			data.size()
		))) {
			return result;
		}
		else {
			if (result)
				result->Release();
			std::cout << "create white texture white to sub is failed\n";
			return nullptr;
		}
	}



	ID3D12Resource* create_black_texture(device* device)
	{
		ID3D12Resource* result = create_color_texture_base(device);
		if (!result) {
			std::cout << "create black texture is failed\n";
			return nullptr;
		}

		std::vector<unsigned char> data(4 * 4 * 4);
		std::fill(data.begin(), data.end(), 0x00);
		for (size_t i = 0; i < 4 * 4; i++)
			data[i * 4] = 0xff;

		if (SUCCEEDED(result->WriteToSubresource(
			0,
			nullptr,
			data.data(),
			4 * 4,
			data.size()
		))) {
			return result;
		}
		else {
			if (result)
				result->Release();
			std::cout << "create black texture white to sub is failed\n";
			return nullptr;
		}
	}


	white_texture_resource::~white_texture_resource()
	{
		if (m_resource)
			m_resource->Release();
	}

	bool white_texture_resource::initialize(device* device)
	{
		m_resource = create_white_texture(device);
		if (m_resource)
			return true;
		else
			return false;

	}

	ID3D12Resource* white_texture_resource::get()
	{
		return m_resource;
	}

	black_texture_resource::~black_texture_resource()
	{
		if (m_resource)
			m_resource->Release();
	}

	bool black_texture_resource::initialize(device* device)
	{
		m_resource = create_black_texture(device);
		if (m_resource)
			return true;
		else
			return false;
	}

	ID3D12Resource* black_texture_resource::get()
	{
		return m_resource;
	}

}