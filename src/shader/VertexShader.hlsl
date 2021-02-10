#include"BasicType.hlsli"


BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,uint instNo : SV_InstanceId)
{

	BasicType output;//�s�N�Z���V�F�[�_�֓n���l

	pos = mul(world, pos);
	if (instNo == 1)
	{
		//���[���h���W�������Ă���
		pos = mul(shadow, pos);
	}

	
	output.svpos = mul(mul(proj, view), pos);//�V�F�[�_�ł͗�D��Ȃ̂Œ���
	output.pos = pos;
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(world, normal);//�@���ɂ����[���h�ϊ����s��
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye));//�����x�N�g��

	output.instNo = instNo;
	
	output.tpos = mul(lightCamera, output.pos);

	return output;
}


float4 shadowVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD) : SV_POSITION
{
	return mul(lightCamera,mul(world,pos));
}