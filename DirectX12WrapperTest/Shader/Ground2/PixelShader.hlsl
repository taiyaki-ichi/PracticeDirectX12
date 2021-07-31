#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	//float3 lightDir = normalize(float3(1.f,1.f,1.f));

	float3 diffuse = float3(1.f, 1.f, 1.f) * saturate(dot(input.normal, normalize(lightDir)));
	float3 ambient = float3(0.01f, 0.01f, 0.01f);

	float3 color = diffuse + ambient;

	return float4(color, 1.f);
}