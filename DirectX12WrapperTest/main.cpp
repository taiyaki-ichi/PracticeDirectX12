//#include"test001.hpp"
//#include"test002.hpp"
//#include"test003.hpp"
//#include"test004.hpp"
//#include"test005.hpp"
//#include"test006.hpp"
//#include"test007.hpp"
#include"test008.hpp"
//#include"test009.hpp"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	DX12::Device device{};
	device.Initialize();

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = 1024;
	resDesc.Height = 1024;
	resDesc.Format = DXGI_FORMAT_R16G16B16A16_TYPELESS;
	resDesc.DepthOrArraySize = 0;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Alignment = 0;

	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	ID3D12Resource* resource = nullptr;
	device.Get()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&resource)
	);

	DX12::DescriptorHeap<DX12::DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
	descriptorHeap.Initialize(&device, 2);

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 0;

		device.Get()->CreateShaderResourceView(resource, &srvDesc, descriptorHeap.GetCPUHandle(0));
	}
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 0;

		device.Get()->CreateShaderResourceView(resource, &srvDesc, descriptorHeap.GetCPUHandle(1));
	}
	
	DXGI_FORMAT_D32_FLOAT

	//return test001::main();
	//return test002::main();
	//return test003::main();
	//return test004::main();
	//return test005::main();
	//return test006::main();
	//return test007::main();
	return test008::main();
	//return test009::main();
}