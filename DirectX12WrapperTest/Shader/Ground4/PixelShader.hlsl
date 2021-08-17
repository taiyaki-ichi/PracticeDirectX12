#include"Header.hlsli"

float4 main(PSInput input) : SV_TARGET
{

	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	float2 depth = shadowMap.Sample(smp, shadowUV);

	if (posFromLightVP.z - 0.005f > depth.x)
	{
		float variance = depth.y - depth.x * depth.x;
		float p = variance / (variance + (posFromLightVP.z - depth.x) * (posFromLightVP.z - depth.x));
		shadowWeight = p;
		shadowWeight *= 0.5;
		shadowWeight += 0.5;
	}

	return float4(1.f, 1.f, 1.f, 1.f) * shadowWeight;
}