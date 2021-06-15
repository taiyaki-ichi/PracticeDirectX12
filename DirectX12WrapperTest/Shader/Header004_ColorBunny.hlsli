

cbuffer World : register(b0) {
	matrix world;
}

cbuffer Color : register(b1) {
	float4 color;
}

cbuffer SceneData : register(b2) {
	matrix view;
	matrix proj;
	float4 lightDir;
};


struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
};