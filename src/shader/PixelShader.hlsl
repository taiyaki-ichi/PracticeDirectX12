#include"BasicType.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ(ベース)
Texture2D<float4> sph:register(t1);//1番スロットに設定されたテクスチャ(乗算)
Texture2D<float4> spa:register(t2);//2番スロットに設定されたテクスチャ(加算)
Texture2D<float4> toon:register(t3);//3番スロットに設定されたテクスチャ(トゥーン)
Texture2D<float> lightDepthTex:register(t4);//ライト深度

SamplerState smp:register(s0);//0番スロットに設定されたサンプラ
SamplerState smpToon:register(s1);//1番スロットに設定されたサンプラ
SamplerComparisonState shadowSmp:register(s2);


cbuffer Material : register(b2) {
	float4 diffuse;//ディフューズ色
	float4 specular;//スペキュラ
	float3 ambient;//アンビエント
};


PixcelOutput main(BasicType input)
{
	
	if (input.instNo == 1)
	{
		PixcelOutput output;
		output.col = float4(0.f, 0.f, 0.f, 1.f);
		output.normal = float4(0.f, 0.f, 0.f, 1.f);
		output.highLum = float4(0.f, 0.f, 0.f, 0.f);
		return output;
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

	float4 result = max(saturate(toonDif//輝度(トゥーン)
		* diffuse//ディフューズ色
		* texColor//テクスチャカラー
		* sph.Sample(smp, sphereMapUV))//スフィアマップ(乗算)
		+ saturate(spa.Sample(smp, sphereMapUV) * texColor//スフィアマップ(加算)
			+ float4(specularB * specular.rgb, 1))//スペキュラー
		, float4(texColor * ambient, 1));//アンビエント


	
	float shadowWeight = 1.0f;
	float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
	float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
	float depthFromLight = lightDepthTex.SampleCmp(
		shadowSmp,
		shadowUV,
		posFromLightVP.z - 0.005f);
	shadowWeight = lerp(0.5f, 1.0f, depthFromLight);

	PixcelOutput output;
	output.col = float4(result.rgb * shadowWeight, result.a);
	output.normal.rgb = float3((input.normal.xyz + 1.0f) / 2.0f);
	output.normal.a = 1;


	float y = dot(float3(0.299f, 0.587f, 0.114f), output.col);
	output.highLum = y > 0.995f ? output.col : 0.0f;
	output.highLum.a = 1.0;

	return output;
}

