#include"BasicType.hlsli"
BasicType main(float4 pos : POSITION, float2 uv : TEXCOORD) {
	BasicType output;//ピクセルシェーダへ渡す値
	output.svpos = pos;
	output.uv = uv;
	return output;
}