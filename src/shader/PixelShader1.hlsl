#include"BasicType.hlsli"

Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
SamplerState smp:register(s0);//0番スロットに設定されたサンプラ


cbuffer Material : register(b2) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
}




//ピクセルシェーダ
float4 main(BasicType input) : SV_TARGET{
	float3 light = normalize(float3(1,-1,1));
	float brightness = saturate(dot(-light, input.normal));
	return float4(brightness, brightness, brightness, 1) * diffuse * tex.Sample(smp, input.uv);
}