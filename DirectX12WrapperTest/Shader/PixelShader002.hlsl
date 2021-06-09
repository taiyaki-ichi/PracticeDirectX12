#include"Header002.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	return tex.Sample(smp,input.uv);
}