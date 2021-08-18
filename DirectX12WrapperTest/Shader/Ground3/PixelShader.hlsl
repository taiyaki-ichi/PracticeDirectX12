#include"Header.hlsli"

static const float PI = 3.141592653589f;

static const int POISSON_DISK_SAMPLE_CNT = 12;
static const float2 POISSON_DISK[12] = {
	float2(0.0191375, 0.635275),
	float2(0.396322, 0.873851),
	float2(-0.588224, 0.588251),
	float2(-0.3404, 0.0154557),
	float2(0.510869, 0.0278614),
	float2(-0.15801, -0.659996),
	float2(0.120268, -0.200636),
	float2(-0.925312, -0.0306309),
	float2(-0.561635, -0.32798),
	float2(0.424297, -0.852628),
	float2(0.923275, -0.191526),
	float2(0.703181, 0.556563)
};

float random(float2 uv) {
	return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float2x2 getRotateFloat2x2(float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return float2x2(c, s, -s, c);
}

float pcf(float3 posFromLightVP, float lightSize, float frustumSize, float bias)
{
	float2x2 rot = getRotateFloat2x2(random(posFromLightVP.xy) * 2 * PI);
	float uvLightSize = lightSize / frustumSize;
	float receiverDistance = posFromLightVP.z - bias;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);

	float sum = 0;
	for (int i = 0; i < POISSON_DISK_SAMPLE_CNT; i++)
	{
		float2 offset = mul(rot, POISSON_DISK[i]) * uvLightSize;
		float occluderDistance = shadowMap.Sample(smp, shadowUV + offset);
		sum += (occluderDistance > receiverDistance) ? 1 : 0;
	}

	return sum / POISSON_DISK_SAMPLE_CNT;
}

float pcss(float3 posFromLightVP, float lightSize, float frustumSize, float bias)
{
	float2x2 rot = getRotateFloat2x2(random(posFromLightVP.xy) * 2 * PI);
	float uvLightSize = lightSize / frustumSize;
	float receiverDistance = posFromLightVP.z - bias;
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	float p33 = sceneData.shadowMapViewProj[2][2];
	float p34 = sceneData.shadowMapViewProj[2][3];

	float averageOccluderDistance = 0;
	int occluderCnt = 0;
	for (int i = 0; i < POISSON_DISK_SAMPLE_CNT; i++)
	{
		float2 offset = mul(rot, POISSON_DISK[i]) * uvLightSize;
		float occluderDistance = shadowMap.Sample(smp, shadowUV + offset);
		if (occluderDistance < receiverDistance) {
			averageOccluderDistance += occluderDistance;
			occluderCnt++;
		}
	}

	if (occluderCnt == 0)
		return 1.f;
	else
		averageOccluderDistance /= occluderCnt;

	float d_r = 1.f / (p33 - receiverDistance * p34);
	float d_o = 1.f / (p33 - averageOccluderDistance * p34);
	float uvFilterSize = (d_r - d_o) / d_o * uvLightSize;

	float sum = 0;
	for (int i = 0; i < POISSON_DISK_SAMPLE_CNT; i++)
	{
		float2 offset = mul(rot, POISSON_DISK[i]) * uvFilterSize;
		float occluderDistance = shadowMap.Sample(smp, shadowUV + offset);
		sum += (occluderDistance > receiverDistance) ? 1 : 0;
	}

	return sum / POISSON_DISK_SAMPLE_CNT;
}



float4 main(PSInput input) : SV_TARGET
{

	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;

	/*
	float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
	float depthFromLight = shadowMap.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f);
	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
	*/

	//shadowWeight = pcf(posFromLightVP, 1.f, 100.f, 0.005f);
	shadowWeight = pcss(posFromLightVP, 1.f, 50.f, 0.005f);

	shadowWeight *= 0.5;
	shadowWeight += 0.5;

	return float4(float3(1.f, 1.f, 1.f) * shadowWeight, 1.f);
}