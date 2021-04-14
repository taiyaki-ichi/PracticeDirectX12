

struct BasicType {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float4 pos:POSITION;//�V�X�e���p���_���W
	float4 normal:NORMAL0;//�@���x�N�g��
	float4 vnormal:NORMAL1;//�@���x�N�g��
	float2 uv:TEXCOORD;//UV�l
	float3 ray:VECTOR;//�x�N�g��
	uint instNo:SV_instanceID;//�C���X�^���X�̔ԍ�
	float4 tpos:TPOS;
};

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;//�r���[�v���W�F�N�V�����s��
	matrix lightCamera;//���C�g�̃J����
	matrix shadow;//�e
	float3 eye;
};

cbuffer Tranceform : register(b1) {
	matrix world;
	matrix bones[512];
}

struct PixcelOutput
{
	float4 col : SV_TARGET0;//�ʏ�́B�܂�F
	float4 normal : SV_TARGET1;//�@��
	float4 highLum : SV_TARGET2;//���P�x
};