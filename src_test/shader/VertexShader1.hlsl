#include"BasicType.hlsli"

//�ϊ����܂Ƃ߂��\����
cbuffer cbuff0 : register(b0) {
	matrix mat;//�ϊ��s��
};

//���_�V�F�[�_
BasicType main(float4 pos : POSITION, float2 uv : TEXCOORD) {
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	output.svpos = mul(mat, pos);
	//output.svpos = pos;
	output.uv = uv;
	return output;
}