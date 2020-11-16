cbuffer cbuff0 : register(b0) {
	matrix mat;//変換行列
};

//頂点シェーダ
float4 main(float4 pos : POSITION) : SV_POSITION{
	//return mul(mat, pos);
	return pos;
}
