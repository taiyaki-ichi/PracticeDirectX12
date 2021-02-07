#include"BasicType.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ(ベース)
Texture2D<float4> sph:register(t1);//1番スロットに設定されたテクスチャ(乗算)
Texture2D<float4> spa:register(t2);//2番スロットに設定されたテクスチャ(加算)
Texture2D<float4> toon:register(t3);//3番スロットに設定されたテクスチャ(トゥーン)
Texture2D<float> lightDepthTex:register(t4);//ライト深度

SamplerState smp:register(s0);//0番スロットに設定されたサンプラ
SamplerState smpToon:register(s1);//1番スロットに設定されたサンプラ


cbuffer Material : register(b1) {
	float4 diffuse;//ディフューズ色
	float4 specular;//スペキュラ
	float3 ambient;//アンビエント
};

float4 main(BasicType input) : SV_TARGET
{
	if (input.instNo == 1)
	{
		return float4(0.f, 0.f, 0.f, 1.f);
	}

	float3 light = normalize(float3(1,-1,1));//光の向かうベクトル(平行光線)
	float3 lightColor = float3(1,1,1);//ライトのカラー(1,1,1で真っ白)

	//ディフューズ計算
	float diffuseB = saturate(dot(-light, input.normal));
	float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));

	//光の反射ベクトル
	float3 refLight = normalize(reflect(light, input.normal.xyz));
	float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);

	//スフィアマップ用UV
	float2 sphereMapUV = input.vnormal.xy;
	sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);

	float4 texColor = tex.Sample(smp, input.uv); //テクスチャカラー

	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
	float depthFromLight = lightDepthTex.Sample(smp,shadowUV);
	if (depthFromLight < posFromLightVP.z-0.001f) {
		shadowWeight = 0.5f;
	}

	float4 result= max(saturate(toonDif//輝度(トゥーン)
		* diffuse//ディフューズ色
		* texColor//テクスチャカラー
		* sph.Sample(smp, sphereMapUV))//スフィアマップ(乗算)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor//スフィアマップ(加算)
			+ float4(specularB * specular.rgb, 1))//スペキュラー
		, float4(texColor * ambient, 1));//アンビエント


	return /*float4(depthFromLight, depthFromLight, depthFromLight, 1.0);*/ float4(result.rgb * shadowWeight, result.a);
}