cbuffer cbuff0 : register(b0) {
	matrix mat;//�ϊ��s��
};

//���_�V�F�[�_
float4 main(float4 pos : POSITION) : SV_POSITION{
	//return mul(mat, pos);
	return pos;
}
