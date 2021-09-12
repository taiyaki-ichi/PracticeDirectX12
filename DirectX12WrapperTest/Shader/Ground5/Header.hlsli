
struct VSInput {
	float4 pos : POSITION;
	float2 uv : TEXCOOD;
};

struct PSInput {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
};

struct GroundData
{
	matrix view;
	matrix proj;
	matrix world;
};

ConstantBuffer<GroundData> groundData : register(b0);
Texture2D<float4> tex: register(t0);
SamplerState smp:register(s0);
