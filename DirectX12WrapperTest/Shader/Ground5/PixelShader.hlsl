#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float2 uv = input.uv * 10;

	return tex.Sample(smp, uv);
}