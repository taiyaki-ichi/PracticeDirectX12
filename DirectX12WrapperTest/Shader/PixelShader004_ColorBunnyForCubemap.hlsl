#include"Header004_ColorBunnyForCubemap.hlsli"

float4 main(GSOutput input) : SV_TARGET
{
	float4 ambient = float4((color / 3.f).rgb,1.f);
	return saturate(dot(lightDir, input.normal)) * color + ambient;
}