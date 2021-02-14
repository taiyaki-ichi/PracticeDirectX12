Texture2D<float4> tex: register(t0);
Texture2D<float4> texNormal: register(t1);
Texture2D<float4> texHighLum: register(t2);
Texture2D<float4> texShrinkHighLum: register(t3);
Texture2D<float4> texShrink: register(t4);
Texture2D<float> texSSAO:register(t5);
Texture2D<float> depthTex: register(t6);

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;//ビュープロジェクション行列
	matrix invProj;
	matrix lightCamera;//ライトのカメラ
	matrix shadow;//影
	float3 eye;
};

SamplerState smp: register(s0);

struct Output
{
	float4 svpos : SV_POSITION;
	float2 uv : TEXCOORD;
};

struct BlurOutput
{
	float4 highLum : SV_TARGET0; //高輝度
	float4 col : SV_TARGET1; //通常のレンダリング結果
};