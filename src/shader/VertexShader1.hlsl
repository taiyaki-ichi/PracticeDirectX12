#include"BasicType.hlsli"

//�ϊ����܂Ƃ߂��\����
cbuffer cbuff0 : register(b0) {
	matrix world;//�ϊ��s��
};
cbuffer cbuff0 : register(b1) {
	matrix viewproj;
};

//���_�V�F�[�_
BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD){
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	output.svpos = mul(mul(viewproj, world), pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}