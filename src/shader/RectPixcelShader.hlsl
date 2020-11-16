
cbuffer cbuff1 : register(b1) {
	float4 color;//色
};

//ピクセルシェーダ
float4 main() : SV_TARGET{
	//return color;
	return float4(0,1,1,1);
}