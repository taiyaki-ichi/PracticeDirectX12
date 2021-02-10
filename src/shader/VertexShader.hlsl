#include"BasicType.hlsli"


BasicType main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD,uint instNo : SV_InstanceId)
{

	BasicType output;//ピクセルシェーダへ渡す値

	pos = mul(world, pos);
	if (instNo == 1)
	{
		//ワールド座標をかけてから
		pos = mul(shadow, pos);
	}

	
	output.svpos = mul(mul(proj, view), pos);//シェーダでは列優先なので注意
	output.pos = pos;
	normal.w = 0;//ここ重要(平行移動成分を無効にする)
	output.normal = mul(world, normal);//法線にもワールド変換を行う
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye));//視線ベクトル

	output.instNo = instNo;
	
	output.tpos = mul(lightCamera, output.pos);

	return output;
}


float4 shadowVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD) : SV_POSITION
{
	return mul(lightCamera,mul(world,pos));
}