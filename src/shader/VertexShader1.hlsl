#include"BasicType.hlsli"

//変換をまとめた構造体
cbuffer cbuff0 : register(b0) {
	matrix world;//変換行列
};
cbuffer cbuff0 : register(b1) {
	matrix viewproj;
};

//頂点シェーダ
BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD){
	BasicType output;//ピクセルシェーダへ渡す値
	output.svpos = mul(mul(viewproj, world), pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}