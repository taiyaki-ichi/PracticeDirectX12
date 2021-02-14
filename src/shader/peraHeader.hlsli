Texture2D<float4> tex: register(t0);
Texture2D<float4> texNormal: register(t1);
Texture2D<float4> texHighLum: register(t2);
Texture2D<float4> texShrinkHighLum: register(t3);
Texture2D<float4> texShrink: register(t4);
Texture2D<float> texSSAO:register(t5);
Texture2D<float> depthTex: register(t6);

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;//�r���[�v���W�F�N�V�����s��
	matrix invProj;
	matrix lightCamera;//���C�g�̃J����
	matrix shadow;//�e
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
	float4 highLum : SV_TARGET0; //���P�x
	float4 col : SV_TARGET1; //�ʏ�̃����_�����O����
};