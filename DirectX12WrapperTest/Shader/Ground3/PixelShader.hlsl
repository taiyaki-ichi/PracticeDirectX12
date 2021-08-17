#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{

	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	float depthFromLight = shadowMap.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f);
	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);

	return float4(1.f, 1.f, 1.f, 1.f) * shadowWeight;
}