#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 diffuse = float3(0.3f, 0.f, 0.f) * saturate(dot(input.normal.xyz, normalize(sceneData.lightDir)));
	float3 ambient = float3(0.5f, 0.0f, 0.0f);

	float3 refLight = normalize(reflect(sceneData.lightDir, input.normal.xyz));
	float3 specular = float3(0.8f, 0.0f, 0.0f) * pow(saturate(dot(refLight, input.ray)), 50.f);

	float3 color = diffuse + ambient + specular;

	
	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	
	float depthFromLight = shadowMap.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f);
	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);

	color *= shadowWeight;

	return float4(color, 1.f);
}