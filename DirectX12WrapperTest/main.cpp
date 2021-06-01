#include"Device.hpp"
#include"CommandList.hpp"
#include"RootSignature/RootSignatureInitializeHelper.hpp"
#include"RootSignature/RootSignatureInitializeTag.hpp"
#include"RootSignature/RootSignature.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"

int main()
{
	using namespace DX12;

	DX12::Device device{};
	device.Initialize();

	DX12::CommandList commandList{};
	commandList.Initialize(&device);

	constexpr auto hoge = GetDescriptorTableTuple<
		DescriptorTableArrayTag<
			DescriptorTableTag<DescriptorRangeTag::CBV,DescriptorRangeTag::CBV>,
			DescriptorTableTag<DescriptorRangeTag::SRV, DescriptorRangeTag::SRV>
		>
	>();

	auto huga = GetDescriptorTableStructArray(hoge);

	RootSignature rootSignature{};
	rootSignature.Initialize<
		DescriptorTableArrayTag<
		DescriptorTableTag<DescriptorRangeTag::CBV, DescriptorRangeTag::CBV>,
		DescriptorTableTag<DescriptorRangeTag::SRV, DescriptorRangeTag::SRV>
		>,
		StaticSamplersTag<StaticSamplerTag::Standard>
	>(&device);

	DescriptorHeap<DescriptorHeapTypeTag::CBV_SRV_UAV> descriptorHeap{};
	descriptorHeap.Initialize(&device, 10);

	return 0;
}