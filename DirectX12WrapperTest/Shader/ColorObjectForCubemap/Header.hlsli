

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float4 lightDir;
	float3 eye;
};


cbuffer ColorObjectData : register(b1) {
	matrix world;
	float4 color;
}


cbuffer CubemapSceneData : register(b2) {
	matrix cubemapView[6];
	matrix cubemapProj;
}

struct VSOutput 
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
	uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};