#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 ambient = float4((color / 3.f).rgb,1.f);
	return saturate(dot(lightDir, input.normal)) * color + ambient;
}