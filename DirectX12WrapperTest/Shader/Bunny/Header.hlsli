
struct SceneData
{
	matrix view;
	matrix proj;
	float4 eye;
	float4 lightDir;
	matrix shadowMapViewProj;
};

struct BunnyData
{
	matrix world[8];
};

ConstantBuffer<SceneData> sceneData : register(b0);
ConstantBuffer<BunnyData> bunnyData : register(b1);
Texture2D<float> shadowMap : register(t0);
SamplerComparisonState shadowSmp:register(s0);

struct PSInput {
	float4 svpos : SV_POSITION;
	float4 pos :POSITION;
	float4 normal :NORMAL;
	float3 ray : RAY;
	float4 tpos:TPOS;
};