#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float2 textureUV = input.uv * 10.f;
	float3 baseDifuuse = float3(0.3, 0.3, 0.3);
	float3 baseAmbient = float3(0.8, 0.8, 0.8);
	float3 baseSpecular = float3(0.2, 0.2, 0.2);

	float3 diffuse = baseDifuuse * saturate(dot(input.normal.xyz, normalize(lightDir)));
	float3 ambient = baseAmbient;

	float3 refLight = normalize(reflect(lightDir, input.normal.xyz));
	float3 specular = baseSpecular * pow(saturate(dot(refLight, input.ray)), 20.f);

	float3 color = diffuse + ambient +specular;

	return float4(color, 1.f);
}