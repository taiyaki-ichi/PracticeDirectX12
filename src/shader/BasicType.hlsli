
/*
struct BasicType {
	float4 svpos:SV_POSITION;//システム用頂点座標
	float2 uv:TEXCOORD;//UV値
};
*/


struct BasicType {
	float4 svpos:SV_POSITION;	//システム用頂点座標
	float4 normal:NORMAL;		//法線ベクトル
	float2 uv:TEXCOORD;		//uv値
};
