
struct SceneData
{
	matrix view;
	matrix proj;
	float4 eye;
	float4 lightDir;
	matrix shadowMapViewProj;
};

struct GroundData
{
	matrix world;
};

ConstantBuffer<SceneData> sceneData : register(b0);
ConstantBuffer<GroundData> groundData : register(b1);
Texture2D<float> shadowMap : register(t0);
SamplerComparisonState shadowSmp:register(s0);
SamplerState smp:register(s1);


struct PSInput {
	float4 svpos : SV_POSITION;
	float4 pos :POSITION;
	float4 tpos:TPOS;
};