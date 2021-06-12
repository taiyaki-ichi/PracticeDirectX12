
cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
};


struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 normal :NORMAL;
};

struct DrawNormalGSOutput
{
	float4 pos : SV_POSITION;
};