#include"BasicType.hlsli"
Texture2D<float4> tex:register(t0);//0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(�x�[�X)
Texture2D<float4> sph:register(t1);//1�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(��Z)
Texture2D<float4> spa:register(t2);//2�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(���Z)
Texture2D<float4> toon:register(t3);//3�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��(�g�D�[��)
Texture2D<float> lightDepthTex:register(t4);//���C�g�[�x

SamplerState smp:register(s0);//0�ԃX���b�g�ɐݒ肳�ꂽ�T���v��
SamplerState smpToon:register(s1);//1�ԃX���b�g�ɐݒ肳�ꂽ�T���v��


cbuffer Material : register(b1) {
	float4 diffuse;//�f�B�t���[�Y�F
	float4 specular;//�X�y�L����
	float3 ambient;//�A���r�G���g
};

float4 main(BasicType input) : SV_TARGET
{
	if (input.instNo == 1)
	{
		return float4(0.f, 0.f, 0.f, 1.f);
	}

	float3 light = normalize(float3(1,-1,1));//���̌������x�N�g��(���s����)
	float3 lightColor = float3(1,1,1);//���C�g�̃J���[(1,1,1�Ő^����)

	//�f�B�t���[�Y�v�Z
	float diffuseB = saturate(dot(-light, input.normal));
	float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//���̔��˃x�N�g��
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//�X�t�B�A�}�b�v�pUV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	float4 texColor = tex.Sample(smp, input.uv); //�e�N�X�`���J���[

	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
	float depthFromLight = lightDepthTex.Sample(smp,shadowUV);
	if (depthFromLight < posFromLightVP.z-0.001f) {
		shadowWeight = 0.5f;
	}

	float4 result= max(saturate(toonDif//�P�x(�g�D�[��)
		* diffuse//�f�B�t���[�Y�F
		* texColor//�e�N�X�`���J���[
		* sph.Sample(smp, sphereMapUV))//�X�t�B�A�}�b�v(��Z)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor//�X�t�B�A�}�b�v(���Z)
			+ float4(specularB * specular.rgb, 1))//�X�y�L�����[
		, float4(texColor * ambient, 1));//�A���r�G���g


	return /*float4(depthFromLight, depthFromLight, depthFromLight, 1.0);*/ float4(result.rgb * shadowWeight, result.a);
}