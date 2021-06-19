
cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float4 lightDir;
	float3 eye;
};


cbuffer World : register(b1) {
	matrix world;
}



struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
	float3 reflect  : REFLECT;
};