#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 diffuse = float3(0.3f, 0.3f, 0.3f) * saturate(dot(input.normal.xyz, normalize(lightDir)));
	float3 ambient = float3(0.2f, 0.2f, 0.2f);

	float3 refLight = normalize(reflect(lightDir, input.normal.xyz));
	float3 specular = float3(0.5f, 0.5f, 0.5f) * pow(saturate(dot(refLight, input.ray)), 10.f);

	float3 color = diffuse + ambient + specular;

	return float4(color, 1.f);
}