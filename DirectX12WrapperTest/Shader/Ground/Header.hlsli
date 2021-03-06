
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
	float3 normal : NORMAL;
};

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	matrix world;
	float3 eye;
	float4 tessRange;
};

Texture2D texHeightMap : register(t0);
Texture2D texNormalMap : register(t1);

SamplerState smp : register(s0);

