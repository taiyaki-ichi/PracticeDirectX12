
cbuffer cbuff0 : register(b0) {
	matrix mat;//�ϊ��s��
};

cbuffer cbuff1 : register(b1) {
	float4 color;//�F
};

//���_�V�F�[�_
float4 RectVSMain(float4 pos : POSITION) {
	return mul(mat, pos);
}

//�s�N�Z���V�F�[�_
float4 RectPSMain(float4 input) : SV_TARGET{
	return color;
}