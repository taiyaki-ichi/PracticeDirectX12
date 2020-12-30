
/*
struct BasicType {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float2 uv:TEXCOORD;//UV�l
};
*/


struct BasicType {
	float4 svpos:SV_POSITION;	//�V�X�e���p���_���W
	float4 normal:NORMAL;		//�@���x�N�g��
	float2 uv:TEXCOORD;		//uv�l
};


cbuffer Material : register(b2) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
}
