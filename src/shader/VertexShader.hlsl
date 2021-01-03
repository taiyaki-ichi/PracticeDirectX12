#include"BasicType.hlsli"

cbuffer SceneData : register(b0) {
	matrix world;//���[���h�ϊ��s��
	matrix view;
	matrix proj;//�r���[�v���W�F�N�V�����s��
	float3 eye;
};




BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{

	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	pos = mul(world, pos);
	output.svpos = mul(mul(proj, view), pos);//�V�F�[�_�ł͗�D��Ȃ̂Œ���
	output.pos = mul(view, pos);
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(world, normal);//�@���ɂ����[���h�ϊ����s��
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye));//�����x�N�g��

	return output;
}