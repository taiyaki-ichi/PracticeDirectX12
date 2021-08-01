
struct VSInput
{
	float4 pos : POSITION;
	float2 uv : TEXCOOD;
};

struct HSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
};

struct HSParameters
{
	float tessFactor[4]			: SV_TessFactor;
	float insideTessFactor[2]			: SV_InsideTessFactor;
};

struct DSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
	float4 normal : NORMAL;
	float3 ray : RAY;
};

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float3 eye;
	float3 lightDir;
};

cbuffer GroundData :register(b1) {
	matrix world;
}

Texture2D<float> texHeightMap : register(t0);
Texture2D<float4> texNormalMap : register(t1);
Texture2D<float4> depthTexture : register(t2);
Texture2D<float4> normalTexture : register(t3);
Texture2D<float> elapsedTimeMap : register(t4);

SamplerState smp : register(s0);

