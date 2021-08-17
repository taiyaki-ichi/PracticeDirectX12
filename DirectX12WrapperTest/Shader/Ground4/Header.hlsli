
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
Texture2D<float2> shadowMap : register(t0);
SamplerState smp:register(s0);


struct PSInput {
	float4 svpos : SV_POSITION;
	float4 pos :POSITION;
	float4 tpos:TPOS;
};