#include"Header003.hlsli"

float4 main(GSOutput input) : SV_TARGET
{
	float3 lightdir = float3(1.f,0.f,0.f);
	lightdir = mul(mul(proj, view), lightdir).xyz;

	float4 color = saturate(dot(input.normal, normalize(lightdir)));

	float4 ambient = float4(0.4f, 0.4f, 0.4f, 1.f);

	return float4(color.xyz, 1.f) + ambient;
}