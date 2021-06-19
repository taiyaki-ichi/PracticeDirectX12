
cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float4 lightDir;
	float3 eye;
};


cbuffer World : register(b1) {
	matrix world;
}

cbuffer Color : register(b2) {
	float4 color;
}

cbuffer CubemapSceneData : register(b3) {
	matrix cubemapView[6];
	matrix cubemapProj;
}


struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
	uint renderTargetArrayIndex : SV_RenderTargetArrayIndex;
};