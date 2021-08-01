
struct PSInput {
	float4 svpos : SV_POSITION;
	float2 uv : TEXCOOD;
};

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float3 eye;
	float3 lightDir;
};

cbuffer SnowData : register(b1) {
	float4 move;
	float4 center;
	float range;
	float rangeR;
	float size;
};

Texture2D<float4> snowTexture : register(t0);

SamplerState smp : register(s0);