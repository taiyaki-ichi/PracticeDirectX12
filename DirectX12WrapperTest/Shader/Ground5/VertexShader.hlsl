#include"Header.hlsli"

PSInput main(VSInput input)
{
	PSInput output;
	output.pos = mul(mul(groundData.proj, groundData.view), mul(groundData.world, input.pos));
	output.uv = input.uv;
	return output;
}