#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 lightDir = normalize(float3(1.f,1.f,0.f));

	float3 diffuse = float3(0.1f, 0.9f, 0.1f) * saturate(dot(input.normal, lightDir));

	return float4(diffuse, 1.f);
}