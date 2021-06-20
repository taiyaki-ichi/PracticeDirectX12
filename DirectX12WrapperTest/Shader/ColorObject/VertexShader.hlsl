#include"Header.hlsli"

VSOutput main(float4 pos : POSITION, float4 normal : NORMAL)
{
	VSOutput output;
	output.pos = mul(mul(proj, view), mul(world, pos));
	output.normal = mul(world, normal);

	return output;
}