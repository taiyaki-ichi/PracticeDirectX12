#include"Header.hlsli"

VSOutput main(float4 pos : POSITION, float4 normal : NORMAL)
{
	VSOutput output;
	pos = mul(world, pos);

	output.pos = mul(mul(proj, view), pos);
	output.normal = mul(world, normal);
	float3 eyeDir = normalize(pos.xyz - eye);
	output.reflect = reflect(eyeDir, mul(world, normal).xyz);
	return output;
}