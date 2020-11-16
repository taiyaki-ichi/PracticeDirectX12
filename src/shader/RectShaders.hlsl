
cbuffer cbuff0 : register(b0) {
	matrix mat;//変換行列
};

cbuffer cbuff1 : register(b1) {
	float4 color;//色
};

//頂点シェーダ
float4 RectVSMain(float4 pos : POSITION) {
	return mul(mat, pos);
}

//ピクセルシェーダ
float4 RectPSMain(float4 input) : SV_TARGET{
	return color;
}