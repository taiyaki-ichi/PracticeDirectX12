

struct BasicType {
	float4 svpos:SV_POSITION;//システム用頂点座標
	float4 pos:POSITION;//システム用頂点座標
	float4 normal:NORMAL0;//法線ベクトル
	float4 vnormal:NORMAL1;//法線ベクトル
	float2 uv:TEXCOORD;//UV値
	float3 ray:VECTOR;//ベクトル
	uint instNo:SV_instanceID;//インスタンスの番号
	float4 tpos:TPOS;
};

cbuffer SceneData : register(b0) {
	matrix world;//ワールド変換行列
	matrix view;
	matrix proj;//ビュープロジェクション行列
	matrix lightCamera;//ライトのカメラ
	matrix shadow;//影
	float3 eye;
};

struct PixcelOutput
{
	float4 col : SV_TARGET0;//通常の。つまり色
	float4 normal : SV_TARGET1;//法線
};