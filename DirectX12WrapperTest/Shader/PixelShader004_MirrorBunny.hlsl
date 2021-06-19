#include"Header004_ColorBunny.hlsli"

TextureCube cubemap:register(t0);

SamplerState smp:register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	return cubemap.Sample(smp,input.reflect) + float4(0.1f,0.1f,0.1f,1.f);
}