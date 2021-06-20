#include"Header.hlsli"

TextureCube cubemap : register(t0);
SamplerState smp:register(s0);

float4 main(VSOutput input) : SV_TARGET
{
	return cubemap.Sample(smp,input.reflect) + float4(0.1f,0.1f,0.1f,1.f);
}