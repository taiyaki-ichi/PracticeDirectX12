#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	return snowTexture.Sample(smp,input.uv);
}