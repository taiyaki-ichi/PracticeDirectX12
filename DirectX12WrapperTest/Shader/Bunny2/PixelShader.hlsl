#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{
	float3 diffuse = float3(0.3f, 0.f, 0.f) * saturate(dot(input.normal.xyz, normalize(sceneData.lightDir)));
	float3 ambient = float3(0.5f, 0.0f, 0.0f);

	float3 refLight = normalize(reflect(sceneData.lightDir, input.normal.xyz));
	float3 specular = float3(0.8f, 0.0f, 0.0f) * pow(saturate(dot(refLight, input.ray)), 50.f);

	float3 color = diffuse;// + ambient + specular;
	
	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	float2 depth = shadowMap.Sample(smp, shadowUV);

	float bias = 0.005f;

	if (posFromLightVP.z - bias > depth.x)
	{
		float variance = depth.y - depth.x * depth.x;
		float p = variance / (variance + (posFromLightVP.z - depth.x) * (posFromLightVP.z - depth.x));
		shadowWeight = p;
		shadowWeight *= 0.5;
		shadowWeight += 0.5;
	}

	color *= shadowWeight;

	//
	color += ambient + specular;

	return float4(color, 1.f);
}