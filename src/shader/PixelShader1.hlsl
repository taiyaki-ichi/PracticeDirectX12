#include"BasicType.hlsli"

Texture2D<float4> tex:register(t0);//0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp:register(s0);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��


cbuffer Material : register(b2) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
}




//�s�N�Z���V�F�[�_
float4 main(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1));
	float brightness = saturate(dot(-light, input.normal));
	return float4(brightness, brightness, brightness, 1) * diffuse * tex.Sample(smp, input.uv);
}