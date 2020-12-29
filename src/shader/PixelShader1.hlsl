#include"BasicType.hlsli"
Texture2D<float4> tex:register(t0);//0番スロットに設定されたテクスチャ
SamplerState smp:register(s0);//0番スロットに設定されたサンプラ

//ピクセルシェーダ
float4 main(BasicType input) : SV_TARGET{
	//return float4(tex.Sample(smp,input.uv));
	return float4(1,0,0,1);
}