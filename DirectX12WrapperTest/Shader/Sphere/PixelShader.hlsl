#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 diffuse = float3(0.3f, 0.f, 0.f) * saturate(dot(input.normal.xyz, normalize(lightDir)));
	float3 ambient = float3(0.5f, 0.0f, 0.0f);

	float3 refLight = normalize(reflect(lightDir, input.normal.xyz));
	float3 specular = float3(0.8f, 0.0f, 0.0f) * pow(saturate(dot(refLight, input.ray)), 50.f);

	float3 color = diffuse + ambient + specular;

	return float4(color, 1.f);
}