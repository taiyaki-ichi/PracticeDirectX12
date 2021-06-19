
cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float4 lightDir;
	float3 eye;
};


cbuffer colorBunnyData : register(b1) {
	matrix world;
	float4 color;
}


struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
	float3 reflect  : REFLECT;
};